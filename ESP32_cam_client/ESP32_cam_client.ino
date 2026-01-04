#include <WiFi.h>
#include "tcp_client.h"
#include "camera.h"
#include "wifi_connection.h"
#include "web_socket.h"
#include "http_server.h"
#include "motor.h"

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);   
  Serial.print("\n\n\n");
  esp_reset_reason_t reason = esp_reset_reason();
  Serial.printf("Reset reason: %d\n", reason);
  Serial.println("\n"); 

  if(!CAM_Init()){
    Serial.println("Camera init failed");
    while (true);
  }
  WIFIC_init();  
  MOTOR_init();
}

void loop() {  
  WIFIC_process();
  
  if (WIFIC_isApActive()) {
    // AP is active, run the HTTP server
    if (!HTTP_SERVER_isRunning()) {
      HTTP_SERVER_init();
      WS_init();
    }
    HTTP_SERVER_process();
    WS_process();
  } else {
    // Connected as a client. Hopefully to the internet. 
    // Shut down HTTP server 
    if (HTTP_SERVER_isRunning()) {
      WS_dispose();
      HTTP_SERVER_stop();          
    }     

    TCPC_Process(); 
  }  
}
