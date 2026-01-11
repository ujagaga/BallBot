#include <WiFi.h>
#include <HTTPClient.h>
#include <Update.h>
#include "tcp_client.h"
#include "camera.h"

static String firmwareDownloadUrl = String(TCP_SERVER_URL) + "/download_firmware";

void HTTPC_fwUpdate() {
    // 1. Stop camera and stabilize memory
    if (CAM_isInitialized()) {
        TCPC_Debug(F("Stopping camera for OTA..."));
        CAM_Stop();
        delay(100);  // allow PSRAM/heap to stabilize
    }

    // 2. Check free heap
    uint32_t freeHeap = ESP.getFreeHeap();
    TCPC_Debug("Free heap before OTA: " + String(freeHeap));
    if (freeHeap < 400 * 1024) {
        TCPC_Debug(F("Not enough free heap to safely start OTA!"));
        return;
    }

    // 3. Start HTTP client
    WiFiClient updateClient;
    HTTPClient http;
    http.setTimeout(15000);
    TCPC_Debug("Starting OTA update from: " + firmwareDownloadUrl);

    if (!http.begin(updateClient, firmwareDownloadUrl)) {
        TCPC_Debug(F("HTTP begin failed"));
        return;
    }

    int httpCode = http.GET();
    if (httpCode != HTTP_CODE_OK) {
        TCPC_Debug("HTTP OTA failed, code: " + String(httpCode));
        http.end();
        return;
    }

    // 4. Get content length
    int contentLength = http.getSize();
    if (contentLength <= 0) {
        TCPC_Debug(F("HTTP Content-Length missing or zero!"));
        http.end();
        return;
    }

    TCPC_Debug("OTA firmware size: " + String(contentLength) + " bytes");

    // 5. Begin OTA
    if (!Update.begin(contentLength)) {
        TCPC_Debug("Update.begin() failed! Error: " + String(Update.errorString()));
        http.end();
        return;
    }

    // 6. Stream OTA with progress
    WiFiClient* stream = http.getStreamPtr();
    const size_t bufSize = 512;
    uint8_t buf[bufSize];
    size_t totalWritten = 0;
    uint32_t lastProgress = 0;

    while (http.connected() && totalWritten < (size_t)contentLength) {
        size_t availableBytes = stream->available();
        if (availableBytes) {
            size_t toRead = availableBytes;
            if (toRead > bufSize) toRead = bufSize;

            int bytesRead = stream->readBytes(buf, toRead);
            if (bytesRead <= 0) continue;

            size_t written = Update.write(buf, bytesRead);
            if (written != (size_t)bytesRead) {
                TCPC_Debug("OTA write failed during streaming!");
                Update.abort();
                http.end();
                return;
            }

            totalWritten += written;

            // Report progress every 5%
            uint32_t progress = (totalWritten * 100) / contentLength;
            if (progress - lastProgress >= 5) {
                lastProgress = progress;
                TCPC_Debug("OTA progress: " + String(progress) + "%");
            }
        }
        delay(10); // yield to Wi-Fi & TCP stack
    }

    // 7. Finish OTA
    if (!Update.end(true)) {
        TCPC_Debug("OTA end failed: " + String(Update.errorString()));
        http.end();
        return;
    }

    if (Update.isFinished()) {
        TCPC_Debug(F("OTA complete. Rebooting..."));
        delay(500);
        ESP.restart();
    } else {
        TCPC_Debug(F("OTA not finished. Something went wrong!"));
    }

    http.end();
}
