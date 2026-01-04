#include <WiFi.h>
#include <Update.h>
#include "config.h"
#include "camera.h"
#include "motor.h"

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
    Serial.printf("No connection to %s:%d, reconnecting...\n", TCP_SERVER_URL, TCP_SERVER_PORT);
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
      String esp32_response = "";

      if (cmd_buffer.equalsIgnoreCase("start")) {
        streamingFlag = true;
        esp32_response = "OK start";
        Serial.println("Streaming started");
      } else if (cmd_buffer.equalsIgnoreCase("stop")) {
        streamingFlag = false;
        esp32_response = "OK stop";
        Serial.println("Streaming stopped");
      }else if (cmd_buffer.startsWith("fwupdate:")) {
        streamingFlag = false;
        uint32_t fwSize = cmd_buffer.substring(9).toInt();
        Serial.printf("Starting firmware update, size=%u bytes\n", fwSize);

        if (!Update.begin(fwSize)) {
          Serial.println("Update.begin failed!");
          client.write((const uint8_t*)"ERR begin\n", 10);
          return;
        } else {
          client.write((const uint8_t*)"OK ready\n", 9);
        }

        uint32_t received = 0;
        uint8_t buf[1024];
        while (received < fwSize) {
          if (client.available()) {
              size_t len = client.readBytes(buf, sizeof(buf));
              Update.write(buf, len);
              received += len;

              // Send progress to server in % (0–100)
              uint8_t percent = (received * 100) / fwSize;
              String prog_msg = "PROGRESS:" + String(percent) + "\n";
              client.write((const uint8_t*)prog_msg.c_str(), prog_msg.length());
          }
          delay(1); // yield to watchdog
        }

        if (Update.end(true)) {
          client.write((const uint8_t*)"DONE\n", 5);
          ESP.restart();
        } else {
          client.write((const uint8_t*)"ERR update\n", 11);
        }
      }else{
        esp32_response = "Unknown command";
        Serial.println("Unknown command");
      }

      if(esp32_response.length() > 1){
        uint32_t len = esp32_response.length();
        uint8_t type = 1;  // text 
        client.write(&type, 1);
        client.write((uint8_t*)&len, 4);
        client.write((const uint8_t*)esp32_response.c_str(), len);
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
      uint8_t imgType = 0;  // image
      uint32_t len = fb->len;

      client.write(&imgType, 1);    // Set message type: image
      client.write((uint8_t*)&len, sizeof(len));
      client.write(fb->buf, fb->len);
      CAM_Dispose(fb);
    }else{
      Serial.println("Camera capture failed");
    }
  }
  
}