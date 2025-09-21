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
static IPAddress stationIP;
static IPAddress apIP(192, 168, 1, 1);
static String apList = "";
static uint32_t apScanTime = 0;
static uint32_t stationModeTime = 0;
static IPAddress dns(8,8,8,8);
static bool accessPointUp = false;


bool WIFIC_isApActive(void){
  return accessPointUp;
}

IPAddress WIFIC_getApIp(void){
  return apIP;
}

/* Returns wifi scan results */
String WIFIC_getApList(void){
  if((millis() - apScanTime) > 3000 ){
    apList = "";
    // WiFi.scanNetworks will return the number of networks found
    int n = WiFi.scanNetworks();
    if (n >0){ 
      apList = WiFi.SSID(0); 
      for (int i = 1; i < n; ++i)
      {      
        apList += "|" + WiFi.SSID(i);  
      }
    }  
    apScanTime = millis();
  }
  return apList;
}

/* Initiates a local AP */
void WIFIC_APMode(void) {
  Serial.println("\nStarting AP mode");
 
  WiFi.begin();
  String ApName = AP_NAME_PREFIX + WiFi.macAddress();
  ApName.toCharArray(myApName, ApName.length() + 1);

  WiFi.mode(WIFI_AP);  
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  if (WiFi.softAP(myApName, "")) {
    Serial.printf("SSID: %s, IP: %s\n", myApName, apIP.toString().c_str());
    statusMessage = "Running in AP mode";
    accessPointUp = true;
    stationModeTime = millis();
  } else {
    Serial.println("Failed to start AP");
  }
}

void WIFIC_stationMode(void) {
  Serial.printf("\n\nTrying STA mode with [%s] and [%s]\r\n", st_ssid, st_pass); 
  
  // Force DHCP + custom DNS
  WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, dns); 
  WiFi.mode(WIFI_STA); 
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

    statusMessage = String("Connected to ") + String(st_ssid);  
    accessPointUp = false;
  }else{
    Serial.println("Could not connect to configured AP");
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
   Serial.println("\n\nReading saved WiFi credentials...");
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

  Serial.printf("SSID: %s, PASS: %s", st_ssid, st_pass);
  // WiFi.persistent(false);     // do not write to NVS anymore
  // WiFi.disconnect(true, true); // true,true = erase old credentials & stop any current connection

  WIFIC_APMode();
}

void WIFIC_process(void) {
  if(accessPointUp && (st_ssid[0] != 0) && ((millis() - stationModeTime) > (AP_MODE_TIMEOUT_S * 1000)) && (WiFi.softAPgetStationNum() == 0)){
    // Time to switch to station mode if possible
    String apList = WIFIC_getApList();
    if (apList.indexOf(String(st_ssid)) != -1) {
      Serial.println("Known SSID found, trying STA...");
      WiFi.softAPdisconnect(true);
      WIFIC_stationMode();
    }     
  }
}

