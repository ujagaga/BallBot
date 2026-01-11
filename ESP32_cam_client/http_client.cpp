#include <WiFi.h>
#include <HTTPClient.h>
#include <Update.h>
#include <ArduinoJson.h>
#include "tcp_client.h"

static bool fwUpdateStarted = false;
static String firmwareDownloadUrl = String(TCP_SERVER_URL) + "/download_firmware";

void HTTPC_fwUpdate() {
  WiFiClient updateClient;
  HTTPClient http;
  http.setTimeout(15000);

  TCPC_Debug("Starting OTA update from: " + firmwareDownloadUrl);

  http.begin(updateClient, firmwareDownloadUrl);
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
        Update.abort();
        return;
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
        TCPC_Debug(String("OTA end error") + Update.errorString());
      }
    } else {
      TCPC_Debug("Not enough space for OTA update!");
    }
  } else {
    TCPC_Debug(String("HTTP OTA failed:") + httpCode);
  }

  http.end();
}