#include <WiFi.h>
#include "tcp_client.h"
#include "camera.h"
#include "wifi_connection.h"
#include "motor.h"
#include "distance.h"

void MAIN_debug(String message){
#ifdef DBG
  Serial.println(message);
#endif
}

void setup() {
#ifdef DBG
  Serial.begin(115200, TX_ONLY);
  Serial.setDebugOutput(true); 
  Serial.print("\n\n\n");
#endif  
     
  esp_reset_reason_t reason = esp_reset_reason();
  MAIN_debug("Reset reason: %d\n\n", reason);

  if(!CAM_Init()){
    MAIN_debug("Camera init failed");
    while (true);
  }
  WIFIC_init();  
  MOTOR_init();
  DISTANCE_init();
}

void loop() {  
  WIFIC_process();
  TCPC_Process();  
}
