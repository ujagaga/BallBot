#include <esp_camera.h>
#include <WiFi.h>

// ====== Camera pin configuration ======
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

// ====== WiFi & Server ======
const char* ssid     = "OhanaBerar";
const char* password = "ohana130315";

const char* server_ip   = "192.168.0.200";  // Raspberry Pi IP
const uint16_t server_port = 5000;

WiFiClient client;
bool streaming = false;  // Streaming state
String cmd_buffer = "";

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();

  // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");

  // Camera config
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.frame_size = FRAMESIZE_SVGA;
  config.pixel_format = PIXFORMAT_JPEG;
  config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
  config.fb_location = CAMERA_FB_IN_PSRAM;
  config.jpeg_quality = 12;
  config.fb_count = 1;

  if (esp_camera_init(&config) != ESP_OK) {
    Serial.println("Camera init failed");
    while (true);
  }

  // Connect TCP
  if (!client.connect(server_ip, server_port)) {
    Serial.println("TCP connection failed");
  } else {
    Serial.println("Connected to server");
  }
}

void loop() {
  if (!client.connected()) {
    Serial.println("Lost connection, reconnecting...");
    if (!client.connect(server_ip, server_port)) {
      delay(1000);
      return;
    }
  }

  // Read commands from server
  while (client.available()) {
    char c = client.read();
    if (c == '\n' || c == '\r') {
      // End of command
      cmd_buffer.trim();  // remove whitespace
      if (cmd_buffer.equalsIgnoreCase("start")) {
        streaming = true;
        Serial.println("Streaming started");
      } else if (cmd_buffer.equalsIgnoreCase("stop")) {
        streaming = false;
        Serial.println("Streaming stopped");
      }
      cmd_buffer = ""; // reset buffer
    } else {
      cmd_buffer += c;
    }
  }

  // Send frame if streaming
  if (streaming) {
    camera_fb_t *fb = esp_camera_fb_get();
    if (!fb) {
      Serial.println("Camera capture failed");
      return;
    }

    uint32_t len = fb->len;
    client.write((uint8_t*)&len, sizeof(len));
    client.write(fb->buf, fb->len);

    esp_camera_fb_return(fb);

    // Limit to ~5fps
    delay(200);
  } else {
    delay(50);  // avoid busy loop
  }
}
