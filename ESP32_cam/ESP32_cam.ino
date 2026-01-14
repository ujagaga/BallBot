// ESP32_cam_client.ino
#include <WiFi.h>
#include "camera.h"
#include "wifi_connection.h"
#include "motor.h"
#include "distance.h"
#include "http_server.h"
#include "http_client.h"


void setup() { 
  WIFIC_init();  
  MOTOR_init();
  DISTANCE_init();  
  HTTP_SERVER_init();
  CAM_Init();
}

void loop() { 
  if(!HTTPC_fwUpdateInProgress()) {
    MOTOR_process();    
  }
  HTTPC_process();  
}
