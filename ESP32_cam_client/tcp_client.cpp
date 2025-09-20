#include <WiFi.h>
#include "config.h"
#include "camera.h"


static WiFiClient client;
static bool streamingFlag = false;
static uint32_t lastTcpConnectAttemptTime = 0;
static String cmd_buffer = "";
static uint32_t lastCaptureTime = 0;


static bool maintain_connection(){
  if (client.connected()){
    return true;
  }

  if((millis() - lastTcpConnectAttemptTime) > 1000){
    Serial.println("No server connection, reconnecting...");
    lastTcpConnectAttemptTime = millis();
    if (!client.connect(TCP_SERVER_URL, TCP_SERVER_PORT)) {
      Serial.println("Failed");
      return false;
    }
    Serial.println("Connected");
  }
  
  return true;
}

void TCPC_Process(){
  if (!maintain_connection()){
    return;
  }

  // Read commands from server
  if(client.available()) {
    char c = client.read();
    if (c == '\n' || c == '\r') {
      // End of command
      cmd_buffer.trim();  // remove whitespace
      if (cmd_buffer.equalsIgnoreCase("start")) {
        streamingFlag = true;
        Serial.println("Streaming started");
      } else if (cmd_buffer.equalsIgnoreCase("stop")) {
        streamingFlag = false;
        Serial.println("Streaming stopped");
      }
      cmd_buffer = ""; // reset buffer
    } else {
      cmd_buffer += c;
    }
  }

  if(streamingFlag && ((millis() - lastCaptureTime) > STREAM_RATE_MS)){
    lastCaptureTime = millis();

    camera_fb_t* fb = CAM_Capture();
    if (fb) {
      uint32_t len = fb->len;
      client.write((uint8_t*)&len, sizeof(len));
      client.write(fb->buf, fb->len);
      CAM_Dispose(fb);
    }else{
      Serial.println("Camera capture failed");
    }
  }
  
}