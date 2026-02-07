#include "camera.h"
#include "distance.h"
#include "http_client.h"
#include "http_server.h"
#include "motor.h"
#include "wifi_connection.h"

void setup() {
  DISTANCE_init();
  MOTOR_init();
  CAM_Init();
  WIFIC_init(true);
  HTTPSRV_init();
}

void loop() {
  if (!HTTPC_fwUpdateInProgress()) {
    MOTOR_process();
  }
  HTTPC_process();
  delay(20);
}
