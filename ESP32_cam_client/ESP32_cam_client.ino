// ESP32_cam_client.ino
#include <WiFi.h>
#include "tcp_client.h"
#include "camera.h"
#include "wifi_connection.h"
#include "motor.h"
#include "distance.h"
#include "http_client.h"

static bool connected = false;

void setup() { 
  WIFIC_init();  
  MOTOR_init();
  DISTANCE_init();  
  CAM_Init();   
}

void loop() {  
  TCPC_Process(); 
  MOTOR_process(); 
}
