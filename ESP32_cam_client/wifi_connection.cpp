/* 
 *  Author: Rada Berar
 *  email: ujagaga@gmail.com
 *  
 *  WiFi connection module. 
 */
#include <WiFi.h>
#include <EEPROM.h>
#include <lwip/init.h>
#include <lwip/dns.h>
#include <lwip/ip_addr.h>
#include "config.h"


static char myApName[32] = {0};    /* Array to form AP name based on read MAC */
static char st_ssid[SSID_SIZE] = {0};    /* SSID to connect to */
static char st_pass[WIFI_PASS_SIZE];    /* Password for the requested SSID */
static unsigned long connectionTimeoutCheck = 0;
static IPAddress stationIP;
static IPAddress apIP(192, 168, 1, 1);
static bool apMode = false;
static uint32_t apModeAttempTime = 0;
static IPAddress dns(8,8,8,8);
// How long to keep AP alive *after* STA connects (in ms)
static const uint32_t AP_KEEP_ALIVE_MS = 15000;   // e.g. 15 seconds
static bool apKeepAlive = false;                  // are we currently in keep-alive mode?
static String connectionStatus = "";  

bool WIFIC_isApMode(void){
  return apMode;
}

String WIFIC_getStatus(void){
  return connectionStatus;
}

void WIFIC_setStatus(String status){
  connectionStatus = status;
}


/* Returns wifi scan results */
String WIFIC_getApList(void){
  String result = "";
  // WiFi.scanNetworks will return the number of networks found
  int n = WiFi.scanNetworks(); 
  if (n >0){ 
    result = WiFi.SSID(0); 
    for (int i = 1; i < n; ++i)
    {      
      result += "|" + WiFi.SSID(i);  
    }
  }
  return result;
}

/* Initiates a local AP */
void WIFIC_APMode(void) {
    Serial.println("\nStarting AP + STA mode");

    WiFi.mode(WIFI_AP_STA);   // Dual mode (AP + Station)
    WiFi.begin();             // Make sure STA is idle for now

    String ApName = AP_NAME_PREFIX + WiFi.macAddress();
    ApName.toCharArray(myApName, ApName.length() + 1);

    WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));

    if (WiFi.softAP(myApName, "")) {
        Serial.printf("AP running: SSID=%s, IP=%s\n",
                      myApName, apIP.toString().c_str());
        apMode = true;
        apKeepAlive = false;
        apModeAttempTime = millis();
        connectionStatus = "Running in AP mode";
    } else {
        Serial.println("Failed to start AP");
        apMode = false;
    }
}


void WIFIC_stationMode(void) {
  Serial.printf("\n\nTrying STA mode with [%s] and [%s]\r\n", st_ssid, st_pass);

  // Force DHCP + custom DNS
  WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, dns);

  WiFi.begin(st_ssid, st_pass);

  int i = 30;
  while ((i > 0) && (WiFi.status() != WL_CONNECTED)) {
    delay(1000);
    Serial.print(".");
    i--;
  }
  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    stationIP = WiFi.localIP();
    IPAddress gateway = WiFi.gatewayIP();

    Serial.printf("IP address: %s, gateway: %s\n",
                  stationIP.toString().c_str(), gateway.toString().c_str());

    connectionStatus = String("Connected to ") + String(st_ssid);
    apMode = false;
  } else {
    WIFIC_APMode();
  }
}

String WIFIC_getStSSID(void){
  return String(st_ssid);
}

void WIFIC_setStSSID(String new_ssid){
  EEPROM.begin(EEPROM_SIZE);
  
  uint16_t addr;
  
  for(addr = 0; addr < new_ssid.length(); addr++){    
    EEPROM.put(addr + SSID_EEPROM_ADDR, new_ssid[addr]);
    st_ssid[addr] = new_ssid[addr];
  }
  EEPROM.put(addr + SSID_EEPROM_ADDR, 0);  
  st_ssid[addr] = 0;
  
  EEPROM.commit();
}

String WIFIC_getStPass(void){
  return String(st_pass);
}

void WIFIC_setStPass(String new_pass){
  EEPROM.begin(EEPROM_SIZE);
  
  uint16_t addr;
  for(addr = 0; addr < new_pass.length(); addr++){   
    EEPROM.put(addr + WIFI_PASS_EEPROM_ADDR, new_pass[addr]);
    st_pass[addr] = new_pass[addr];
  }
  EEPROM.put(addr + WIFI_PASS_EEPROM_ADDR, 0);
  st_pass[addr] = 0;
    
  EEPROM.commit();
}

IPAddress WIFIC_getStIP(void){
  return stationIP;
}

void WIFIC_init(void){  
   /* Read settings from EEPROM */
  EEPROM.begin(EEPROM_SIZE);
  uint16_t i = 0;
  
  do{
    EEPROM.get(i + WIFI_PASS_EEPROM_ADDR, st_pass[i]);
    if((st_pass[i] < 32) || (st_pass[i] > 126)){
      /* Non printable character */
      break;
    }
    i++;
  }while(i < WIFI_PASS_SIZE);
  st_pass[i] = 0;

  i = 0;
  do{
    EEPROM.get(i + SSID_EEPROM_ADDR, st_ssid[i]);
    if((st_ssid[i] < 32) || (st_ssid[i] > 126)){
      /* Non printable character */
      break;
    }
    i++;
  }while(i < SSID_SIZE);
  st_ssid[i] = 0;

  WIFIC_APMode();
}

void WIFIC_process(void) {
  // ----- Station connection logic -----
  if (st_ssid[0] != 0 && WiFi.status() != WL_CONNECTED) {
      // Attempt STA connection if AP is still running
      if ((millis() - apModeAttempTime) > (AP_MODE_TIMEOUT_S * 1000)) {
          String apList = WIFIC_getApList();
          if (apList.indexOf(String(st_ssid)) != -1) {
              Serial.println("Known SSID found, trying STA...");
              WIFIC_stationMode();
          }
          apModeAttempTime = millis(); // reset scan timer
      }
  }

  // ----- Keep-alive / shutdown logic -----
  if (WiFi.status() == WL_CONNECTED && apMode) {
    // Start keep-alive timer if not already started
    if (!apKeepAlive) {
        apKeepAlive = true;
        apModeAttempTime = millis();
        Serial.printf("STA connected (%s). "
                      "Keeping AP alive for %lu ms...\n",
                      WiFi.localIP().toString().c_str(),
                      (unsigned long)AP_KEEP_ALIVE_MS);
    } else if ((millis() - apModeAttempTime) > AP_KEEP_ALIVE_MS) {
        // Time to shut down AP
        Serial.println("Keep-alive expired — shutting down AP.");
        WiFi.softAPdisconnect(true);
        apMode = false;
        apKeepAlive = false;
    }
  }
}

