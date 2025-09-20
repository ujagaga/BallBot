#ifndef CONFIG_H
#define CONFIG_H

#define TCP_SERVER_URL          "ujagaga.tplinkdns.com"
#define TCP_SERVER_URL          "192.168.0.200"
#define TCP_SERVER_PORT         5100

// Capture and send an image every few ms
#define STREAM_RATE_MS          200



#define RELAY_1_PIN             16   
#define RELAY_2_PIN             17  
#define RELAY_3_PIN             18  
#define RELAY_4_PIN             19 

// 5 minutes to start the OTA update. If not, stop the service.
#define UPDATE_TIMEOUT          (300000ul)
#define MQTT_SERVER             "ujagaga.tplinkdns.com"
#define MQTT_PORT		50000	
#define MQTT_USER               "rada"
#define MQTT_PASS               "gagisa"
#define MQTT_TOPIC              "ballbot"
#define REPORT_URL              "ujagaga.tplinkdns.com:8080"

#define AP_NAME_PREFIX          "ballbot_"

#define WIFI_PASS_EEPROM_ADDR   (0)
#define WIFI_PASS_SIZE          (32)
#define SSID_EEPROM_ADDR        (WIFI_PASS_EEPROM_ADDR + WIFI_PASS_SIZE)
#define SSID_SIZE               (32)
#define EEPROM_SIZE             (WIFI_PASS_SIZE + SSID_SIZE)   

#endif
