#include <WiFi.h>
#include <ArduinoJson.h>
#include "camera.h"
#include "tcp_client.h"
#include "motor.h"
#include "wifi_connection.h"

// ---------------- CONFIG ----------------
#define MAX_CMD_LEN        512
#define IMAGE_CHUNK_SIZE  1024   // Safe chunk size for ESP32 TCP

static constexpr size_t JSON_BUF = 512;
static constexpr uint32_t HEARTBEAT_MS = 2000;
static constexpr uint32_t TCP_RETRY_MS = 2000;

// ---------------- STATE -----------------
static WiFiClient client;
static bool streamingFlag = true;
static uint32_t lastCaptureTime = 0;
static uint32_t lastHeartbeat = 0;
static String cmd_buffer = "";
static uint32_t lastTcpAttempt = 0;

// ---------------- INTERNAL HELPERS ----------------
static bool sendBuffer(const uint8_t* data, size_t len) {
    size_t sent = 0;

    while (sent < len) {
        size_t avail = client.availableForWrite();
        if (avail == 0) {
            return false; // would block → abort
        }

        size_t chunk = min(avail, min((size_t)IMAGE_CHUNK_SIZE, len - sent));
        size_t w = client.write(data + sent, chunk);
        if (w == 0) {
            return false;
        }
        sent += w;
    }
    return true;
}

// ---------------- TCP PROCESS ----------------
void TCPC_Process() {

    // --- 0. WiFi must be connected ---
    if (!WIFIC_connected()) {
        client.stop();
        return;
    }

    // --- 1. Maintain TCP connection ---
    if (!client.connected()) {
        uint32_t now = millis();
        if (now - lastTcpAttempt < TCP_RETRY_MS)
            return;

        lastTcpAttempt = now;
        client.stop();

        if (!client.connect(TCP_SERVER_URL, TCP_SERVER_PORT))
            return;

        streamingFlag = true;

        TCPC_Debug(CAM_isInitialized() ? "Camera initialized" : "Camera failure");
    }

    // --- 2. Read JSON commands ---
    while (client.available()) {
        char c = client.read();

        if (c == '\n') {
            cmd_buffer.trim();

            if (!cmd_buffer.isEmpty()) {
                StaticJsonDocument<JSON_BUF> doc;
                String response = "OK";

                DeserializationError err = deserializeJson(doc, cmd_buffer);
                if (err) {
                    response = "ERR json";
                } else if (!doc["cmd"].is<const char*>()) {
                    response = "ERR cmd type";
                } else {
                    const char* cmd = doc["cmd"];

                    // -------- STREAM --------
                    if (!strcmp(cmd, "stream")) {
                        if (doc.containsKey("enable"))
                            streamingFlag = doc["enable"];
                        else
                            response = "ERR stream param";
                    }

                    // -------- LED --------
                    else if (!strcmp(cmd, "light")) {
                        if (doc.containsKey("value"))
                            CAM_light(doc["value"]);
                        else
                            response = "ERR light param";
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
                uint8_t type = 1;
                uint32_t len = ack.length();

                if (client.availableForWrite() >= (1 + 4 + len)) {
                    client.write(&type, 1);
                    client.write((uint8_t*)&len, 4);
                    client.write((const uint8_t*)ack.c_str(), len);
                }
            }

            cmd_buffer = "";
        } else {
            cmd_buffer += c;
            if (cmd_buffer.length() >= MAX_CMD_LEN)
                cmd_buffer = "";
        }
    }

    uint32_t now = millis();

    // --- 3. Heartbeat ---
    if (!streamingFlag && (now - lastHeartbeat > HEARTBEAT_MS)) {
        lastHeartbeat = now;
        TCPC_Debug("heartbeat");
    }

    // --- 4. Stream frames (non-blocking, chunked) ---
    if (streamingFlag && (now - lastCaptureTime > STREAM_RATE_MS)) {
        lastCaptureTime = now;

        camera_fb_t* fb = CAM_Capture();
        if (!fb)
            return;

        uint8_t type = 0;
        uint32_t len = fb->len;
        size_t total = 1 + 4 + len;

        if (client.availableForWrite() < total) {
            CAM_Dispose(fb); // drop frame
            return;
        }

        if (!client.write(&type, 1) ||
            !client.write((uint8_t*)&len, 4) ||
            !sendBuffer(fb->buf, fb->len)) {
            client.stop();
        }

        CAM_Dispose(fb);
    }
}

// ---------------- DEBUG HELPER ----------------
bool TCPC_Debug(String msg) {
    if (!client.connected() || msg.isEmpty())
        return false;

    uint8_t type = 2;
    uint32_t len = msg.length();

    if (client.availableForWrite() < (1 + 4 + len))
        return false;

    client.write(&type, 1);
    client.write((uint8_t*)&len, 4);
    client.write((const uint8_t*)msg.c_str(), len);
    return true;
}
