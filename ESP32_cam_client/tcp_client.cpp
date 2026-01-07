#include <WiFi.h>
#include <ArduinoJson.h>
#include "camera.h"
#include "tcp_client.h"
#include "motor.h"
#include "wifi_connection.h"

// ---------------- CONFIG ----------------
static constexpr size_t JSON_BUF = 256;
static constexpr uint32_t HEARTBEAT_MS = 2000;

// ---------------- STATE -----------------
static WiFiClient client;
static bool streamingFlag = true;
static uint32_t lastCaptureTime = 0;
static uint32_t lastHeartbeat = 0;
static String cmd_buffer = "";
static uint32_t lastConnectAttemptTime = 0;


// ---------------- TCP PROCESS ----------------
void TCPC_Process() {
    // --- 0. Make sure wifi is available ---
    if(!WIFIC_connected()){
        lastConnectAttemptTime = 0;
        return;
    }
    if(lastConnectAttemptTime == 0){
        lastConnectAttemptTime = millis();
        return;
    }
    if((millis() - lastConnectAttemptTime) < 200){
        return;
    }

    // --- 1. Maintain TCP connection ---
    if (!client.connected()) {
        if (client) client.stop();

        if (!client.connect(TCP_SERVER_URL, TCP_SERVER_PORT)) {
            return;
        }

        streamingFlag = true;
        TCPC_Debug("Connection established");
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

                        if (doc.containsKey("speed")) {
                            MOTOR_move(doc["speed"]);
                        }
                        else if (doc.containsKey("stop")) {
                            MOTOR_stop();
                        }
                        else if (doc.containsKey("timeout")) {
                            MOTOR_setCmdTimeout(doc["timeout"]);
                        }
                        else {
                            response = "ERR motor param";
                        }
                    }

                    // -------- SERVO --------
                    else if (!strcmp(cmd, "servo")) {

                        if (doc.containsKey("id") && doc.containsKey("angle")) {
                            int id = doc["id"];
                            int angle = doc["angle"];

                            if (id >= 0 && id <= 2 && angle >= 0 && angle <= 180) {
                                MOTOR_setServo(id, angle);
                            } else {
                                response = "ERR servo range";
                            }
                        }
                        else {
                            response = "ERR servo param";
                        }
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
