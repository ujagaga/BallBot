// wifi_connection.cpp
#include <WiFi.h>
#include "wifi_connection.h"

void WIFIC_init() {
    WiFi.mode(WIFI_STA);  
    WiFi.begin(WIFI_SSID, WIFI_PASS);  
}

bool WIFIC_connected() {
    return (WiFi.status() == WL_CONNECTED);
}
