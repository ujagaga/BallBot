#include <WiFi.h>
#include "tcp_client.h"
#include "camera.h"
#include "wifi_connection.h"
#include "web_socket.h"
#include "http_server.h"
#include "ota.h"

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();  

  if(!CAM_Init()){
    Serial.println("Camera init failed");
    while (true);
  }

  WIFIC_init();
  WS_init();
}

void loop() {
  if(OTA_updateInProgress()){
    OTA_process();
  }else{
    TCPC_Process();  
    WIFIC_process();
    WS_process();

    // If AP is active, run the HTTP server
    if (WIFIC_isApMode()) {
      if (!HTTP_SERVER_isRunning()) {
          HTTP_SERVER_init();
      }
      HTTP_SERVER_process();
    } else {
      // Shut down HTTP server if AP is off
      if (HTTP_SERVER_isRunning()) {
          HTTP_SERVER_stop();
      }
    }
  }
}
