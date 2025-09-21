#ifndef WIFI_CONNECTION_H
#define WIFI_CONNECTION_H

#include <WiFiClient.h>


extern void WIFIC_init(void);
extern void WIFIC_setStSSID(String new_ssid);
extern void WIFIC_setStPass(String new_pass);
extern String WIFIC_getApList(void);
extern IPAddress WIFIC_getApIp(void);
extern String WIFIC_getStSSID(void);
extern String WIFIC_getStPass(void);
extern void WIFIC_process(void);
extern bool WIFIC_isApActive(void);

#endif
