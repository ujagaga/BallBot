// ESP32_cam_client.ino
#include <WiFi.h>
#include "tcp_client.h"
#include "camera.h"
#include "wifi_connection.h"
#include "motor.h"
#include "distance.h"

static bool connected = false;

void setup() { 
  WIFIC_init();  
  MOTOR_init();
  DISTANCE_init();  
  CAM_Init(); 
  pinMode(BLDC_TAHO_PIN, INPUT_PULLDOWN); 
  delay(100);
  pinMode(BLDC_TAHO_PIN, INPUT_PULLUP); 
  delay(100);
   pinMode(BLDC_TAHO_PIN, INPUT_PULLDOWN); 
  delay(100);
  pinMode(BLDC_TAHO_PIN, INPUT_PULLUP); 
  delay(100);
   pinMode(BLDC_TAHO_PIN, INPUT_PULLDOWN); 
  delay(100);
  pinMode(BLDC_TAHO_PIN, INPUT_PULLUP); 
  delay(100);
   pinMode(BLDC_TAHO_PIN, INPUT_PULLDOWN); 
  delay(100);
  pinMode(BLDC_TAHO_PIN, INPUT_PULLUP); 
}

void loop() {  
  TCPC_Process();  
  MOTOR_process();  
}
