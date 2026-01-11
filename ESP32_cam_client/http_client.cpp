#include <WiFi.h>
#include <HTTPClient.h>
#include <Update.h>
#include <ArduinoJson.h>
#include "tcp_client.h"

static bool fwUpdateStarted = false;

void HTTPC_fwUpdate() {
  WiFiClient updateClient;
  HTTPClient http;

  TCPC_Debug("Starting OTA update from: " + String(TCP_SERVER_URL));

  http.begin(updateClient, TCP_SERVER_URL);
  int httpCode = http.GET();

  if (httpCode == HTTP_CODE_OK) {
    int contentLength = http.getSize();
    bool canBegin = Update.begin(contentLength);

    if (canBegin) {
      WiFiClient* stream = http.getStreamPtr();
      size_t written = Update.writeStream(*stream);

      if (written == contentLength) {
        TCPC_Debug("OTA written successfully, size: " + String(written));
      } else {
        TCPC_Debug("OTA write failed! Written only: " + String(written) + " / " + String(contentLength));
      }

      if (Update.end()) {
        if (Update.isFinished()) {
          TCPC_Debug("OTA update complete. Rebooting...");
          delay(500);
          ESP.restart();
        } else {
          TCPC_Debug("OTA not finished. Something went wrong!");
        }
      } else {
        TCPC_Debug("OTA end error (%d): %s\n", Update.getError(), Update.errorString());
      }
    } else {
      TCPC_Debug("Not enough space for OTA update!");
    }
  } else {
    TCPC_Debug("HTTP OTA failed, code: %d\n", httpCode);
  }

  http.end();
}