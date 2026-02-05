#include "camera.h"
#include "distance.h"
#include "http_client.h"
#include "http_server.h"
#include "motor.h"
#include "wifi_connection.h"
#include <WiFi.h>

void setup() {
  DISTANCE_init();
  MOTOR_init();
  // Do not edit past this point to avoid camera init failure
  CAM_Init();
  WIFIC_init(true);
  HTTPSRV_init();
}

void loop() {
  if (!HTTPC_fwUpdateInProgress()) {
    MOTOR_process();
  }
  HTTPC_process();
  delay(100);
}
