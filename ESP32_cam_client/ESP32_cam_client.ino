#include <WiFi.h>
#include "tcp_client.h"
#include "camera.h"

const char* ssid     = "OhanaCode_2G";
const char* password = "0hana0ffice";


void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();

  // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");

  if(!CAM_Init()){
    Serial.println("Camera init failed");
    while (true);
  }
}

void loop() {
  TCPC_Process();  
}
