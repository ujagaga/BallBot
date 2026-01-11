#include <WiFi.h>
#include <ArduinoJson.h>
#include "camera.h"
#include "tcp_client.h"
#include "motor.h"
#include "wifi_connection.h"
#include "http_client.h"

// ---------------- CONFIG ----------------
static constexpr size_t JSON_BUF = 512;
static constexpr uint32_t HEARTBEAT_MS = 2000;

// ---------------- STATE -----------------
static WiFiClient client;
static bool streamingFlag = true;
static uint32_t lastCaptureTime = 0;
static uint32_t lastHeartbeat = 0;
static String cmd_buffer = "";
static uint32_t lastConnectAttemptTime = 0;
static bool fwUpdateStarted = false;


bool TCPC_IsFwUpdateInProgress()
{
    return fwUpdateStarted;
}


void fwUpdateTask(void* arg) {    
    HTTPC_fwUpdate();
    fwUpdateStarted = false;
    vTaskDelete(NULL);
    CAM_Init();
    streamingFlag = true;
}

// ---------------- TCP PROCESS ----------------
void TCPC_Process() {
    // --- 0. Make sure wifi is available ---
    if(!WIFIC_connected()){
        lastConnectAttemptTime = 0;
        // client.stop();
        return;
    }
    if(lastConnectAttemptTime == 0){
        lastConnectAttemptTime = millis();
        return;
    }
    if((millis() - lastConnectAttemptTime) < HEARTBEAT_MS){
        return;
    }

    // --- 1. Maintain TCP connection ---
    if (!client.connected()) {
        if (client) client.stop();

        if (!client.connect(TCP_SERVER_URL, TCP_SERVER_PORT)) {
            return;
        }

        streamingFlag = true;
        if(CAM_isInitialized()){
            TCPC_Debug("Camera initialized");
        }else{
            TCPC_Debug("Camera failure");
        }
    }

    // --- 2. Read JSON commands ---
    while (client.available()) {
        char c = client.read();

        if (c == '\n') {
            cmd_buffer.trim();

            if (cmd_buffer.length() > 0) {

                StaticJsonDocument<JSON_BUF> doc;
                DeserializationError err = deserializeJson(doc, cmd_buffer);

                String response = "OK";

                if (err) {
                    response = "ERR json";
                }
                else {
                    const char* cmd = doc["cmd"];

                    if (!cmd) {
                        response = "ERR no cmd";
                    }

                    // -------- FIRMWARE UPDATE --------
                    else if (!strcmp(cmd, "fwUpdate")) {
                        response = "ERR Not implemented";
                        // if (!fwUpdateStarted) {                           
                        //     fwUpdateStarted = true;
                        //     xTaskCreate(fwUpdateTask,"fw_update", 8192, nullptr, 1, nullptr);
                        // }
                    }   

                    // -------- STREAM --------
                    else if (!strcmp(cmd, "stream")) {
                        if (doc.containsKey("enable")) {
                            streamingFlag = doc["enable"];
                        } else {
                            response = "ERR stream param";
                        }
                    }                    

                    // -------- MOTOR --------
                    else if (!strcmp(cmd, "motor")) {
                        if (doc.containsKey("distance")) {
                            uint32_t dist = doc["distance"];
                            int speed = doc.containsKey("speed") ? doc["speed"] : 100;
                            bool keepDir = doc.containsKey("keepDir") ? doc["keepDir"] : true;
                            MOTOR_moveToDistance(dist, speed, keepDir);
                        } else {
                            response = "ERR motor param";
                        }
                    }

                    // -------- SERVOS --------
                    else if (!strcmp(cmd, "servoClaw")) {
                        MOTOR_setClawServo(doc["angle"] | 0);
                    }
                    else if (!strcmp(cmd, "servoArm")) {
                        MOTOR_setArmServo(doc["angle"] | 0);
                    }
                    else if (!strcmp(cmd, "servoSteer")) {
                        MOTOR_setSteerServo(doc["angle"] | 0);
                    }
                    else if (!strcmp(cmd, "servoClawIncrement")) {
                        MOTOR_incrementClawServo(doc["angle"] | 0);
                    }
                    else if (!strcmp(cmd, "servoArmIncrement")) {
                        MOTOR_incrementArmServo(doc["angle"] | 0);
                    }
                    else if (!strcmp(cmd, "servoSteerIncrement")) {
                        MOTOR_incrementSteerServo(doc["angle"] | 0);
                    }

                    else {
                        response = "ERR unknown cmd";
                    }
                }

                // --- Send ACK ---
                String ack = cmd_buffer + ":" + response;
                uint8_t type = 1; // text
                uint32_t len = ack.length();

                client.write(&type, 1);
                client.write((uint8_t*)&len, 4);
                client.write((const uint8_t*)ack.c_str(), len);
            }

            cmd_buffer = "";
        }
        else {
            cmd_buffer += c;
        }
    }

    uint32_t now = millis();

    // --- 3. Heartbeat when stream paused ---
    if (!streamingFlag && (now - lastHeartbeat > HEARTBEAT_MS)) {
        lastHeartbeat = now;
        TCPC_Debug("heartbeat");
    }

    // --- 4. Stream frames ---
    if (streamingFlag && (now - lastCaptureTime > STREAM_RATE_MS)) {
        lastCaptureTime = now;

        camera_fb_t* fb = CAM_Capture();
        if (fb) {
            uint8_t type = 0; // image
            uint32_t len = fb->len;

            client.write(&type, 1);
            client.write((uint8_t*)&len, sizeof(len));
            client.write(fb->buf, fb->len);

            CAM_Dispose(fb);
        }
    }
}

// ---------------- DEBUG HELPER ----------------
bool TCPC_Debug(String msg) {
    if (client.connected() && msg.length() > 0) {
        uint8_t type = 2;
        uint32_t len = msg.length();

        client.write(&type, 1);
        client.write((uint8_t*)&len, 4);
        client.write((const uint8_t*)msg.c_str(), len);
        return true;
    }
    return false;
}
