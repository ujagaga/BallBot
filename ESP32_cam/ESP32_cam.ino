#include "esp_camera.h"
#include <WiFi.h>
#include "camera.h"
#include "wifi_connection.h"
#include "http_server.h"
#include "http_client.h"
#include "distance.h"
#include "motor.h"

void setup() {     
  DISTANCE_init();
  MOTOR_init();
  // Do not edit past this point to avoid camera init failure 
  CAM_Init(); 
  WIFIC_init();  
  HTTPSRV_init();  
}

void loop() {
  if(!HTTPC_fwUpdateInProgress()) {
    MOTOR_process();    
  }
  HTTPC_process(); 
  // 10ms minimum delay to allow camera to sample image
  delay(40);
}
