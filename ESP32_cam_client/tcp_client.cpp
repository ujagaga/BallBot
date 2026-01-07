#include <WiFi.h>
#include "camera.h"
#include "tcp_client.h"

// --- Globals ---
static WiFiClient client;
static bool streamingFlag = false;
static uint32_t lastCaptureTime = 0;
static String cmd_buffer = "";

// --- TCP Connection / Streaming ---
void TCPC_Process() {
    // --- 1. Maintain TCP connection ---
    if (!client.connected()) {
        if (client) client.stop();  // close old client
        if (client.connect(TCP_SERVER_URL, TCP_SERVER_PORT)) {
            streamingFlag = true;  // start streaming immediately
            TCPC_Debug("Connection established...");
        } else {
            delay(500); // wait and retry
            return;
        }
    }

    // --- 2. Read commands from server ---
    while (client.available()) {
        char c = client.read();
        if (c == '\n' || c == '\r') {
            cmd_buffer.trim();
            if (cmd_buffer.length() > 0) {
                if (cmd_buffer.equalsIgnoreCase("stop")) streamingFlag = false;
                else if (cmd_buffer.equalsIgnoreCase("start")) streamingFlag = true;
                
                // send ACK to server
                String resp = "OK:" + cmd_buffer;
                uint32_t len = resp.length();
                uint8_t type = 1;  // text
                client.write(&type, 1);
                client.write((uint8_t*)&len, 4);
                client.write((const uint8_t*)resp.c_str(), len);
            }
            cmd_buffer = "";
        } else {
            cmd_buffer += c;
        }
    }

    // --- 3. Capture and send frames ---
    if (streamingFlag && (millis() - lastCaptureTime > STREAM_RATE_MS)) {
        lastCaptureTime = millis();
        camera_fb_t* fb = CAM_Capture();
        if (fb) {
            uint8_t type = 0;  // image
            uint32_t len = fb->len;
            client.write(&type, 1);
            client.write((uint8_t*)&len, sizeof(len));
            client.write(fb->buf, fb->len);
            CAM_Dispose(fb);
        }
    }
}

// --- Debug helper ---
bool TCPC_Debug(String msg) {
    if (client.connected() && msg.length() > 0) {
        uint8_t type = 2;  // debug
        uint32_t len = msg.length();
        client.write(&type, 1);
        client.write((uint8_t*)&len, 4);
        client.write((const uint8_t*)msg.c_str(), len);
        return true;
    }
    return false;
}
