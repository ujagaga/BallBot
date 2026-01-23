// http_client.cpp
#include <WiFi.h>
#include <HTTPClient.h>
#include <HTTPUpdate.h>
#include "camera.h"
#include "logger.h"
#include "http_server.h"

static bool fwUpdateInProgress = false; 
static bool fwUpdateRequested = false;
static char _firmwareDownloadUrl[255] = {0};
static char _logCallbackUrl[255] = {0};

bool HTTPC_fwUpdateInProgress() {
    return fwUpdateInProgress;
}

/*
 * Send a single log message.
 * Logging is intentionally disabled during OTA streaming.
 */
void sendLog(char* logMsg)
{
    if (logMsg == nullptr) {
        return;
    }

    if (fwUpdateInProgress) {
        return;
    }

    if (_logCallbackUrl[0] == '\0') {
        return;
    }

    HTTPClient http;
    http.setTimeout(3000);

    if (!http.begin(_logCallbackUrl)) {
        return;
    }

    http.addHeader("Content-Type", "text/plain");
    http.POST((uint8_t*)logMsg, strlen(logMsg));
    http.end();
}


void HTTPC_fwUpdateRequest(char* fwDownloadUrl, char* logUrl){
    if (fwUpdateRequested || fwUpdateInProgress) {
        return;
    }

    if (fwDownloadUrl != nullptr){
        strncpy(_firmwareDownloadUrl, fwDownloadUrl, sizeof(_firmwareDownloadUrl) - 1);
        _firmwareDownloadUrl[sizeof(_firmwareDownloadUrl) - 1] = '\0';
    } 
    if (logUrl != nullptr){
        strncpy(_logCallbackUrl, logUrl, sizeof(_logCallbackUrl) - 1);
        _logCallbackUrl[sizeof(_logCallbackUrl) - 1] = '\0';
    } 

    fwUpdateRequested = true;
}

void HTTPC_fwUpdate(){
    if (_firmwareDownloadUrl[0] == '\0') {
        sendLog("OTA failed: empty firmware URL");
        fwUpdateInProgress = false;
        return;
    }

    if (fwUpdateInProgress) {
        return;
    }    

    fwUpdateInProgress = true;    

    sendLog("Disabling camera");

    /* Stop camera to free PSRAM/heap */
    if (CAM_isInitialized()) {
        CAM_Stop();
        delay(100);
    }

    sendLog("Disabling HTTP servers");
    HTTP_SERVER_stop();

    uint32_t freeHeap = ESP.getFreeHeap();
    if (freeHeap < 60 * 1024) {
        sendLog("Warning: low heap before OTA");
    }

    sendLog("OTA update started");
    WiFiClient updateClient;
    t_httpUpdate_return ret = httpUpdate.update(updateClient, _firmwareDownloadUrl);

    switch (ret) {
        case HTTP_UPDATE_FAILED:
            sendLog("OTA Failed");
            break;
        case HTTP_UPDATE_NO_UPDATES:
            sendLog("No Update");
            break;
        case HTTP_UPDATE_OK:
            sendLog("Update OK"); // May not print as it reboots automatically
            break;
    }

    fwUpdateInProgress = false;
    _logCallbackUrl[0] = '\0';
    CAM_Init();
}


void HTTPC_process(void){
    if(fwUpdateRequested){
        fwUpdateRequested = false;
        HTTPC_fwUpdate();
    }
}
