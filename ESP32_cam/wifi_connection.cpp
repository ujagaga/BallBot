// wifi_connection.cpp
#include "wifi_connection.h"
#include <WiFi.h>
#include <esp_wifi.h>

int chooseBestChannel() {
  int channelScore[14] = {0};

  WiFi.mode(WIFI_STA);
  WiFi.disconnect(true);
  delay(100);

  int n = WiFi.scanNetworks(false, true);

  for (int i = 0; i < n; i++) {
    int ch = WiFi.channel(i);
    int rssi = WiFi.RSSI(i);

    if (ch >= 1 && ch <= 13) {
      channelScore[ch] += abs(rssi);
    }
  }

  int preferred[] = {1, 6, 11};
  int bestChannel = preferred[0];
  int bestScore = 999999;

  for (int i = 0; i < 3; i++) {
    int ch = preferred[i];
    if (channelScore[ch] < bestScore) {
      bestScore = channelScore[ch];
      bestChannel = ch;
    }
  }

  return bestChannel;
}

static void setup_ap() {
  int channel = chooseBestChannel();
  WiFi.mode(WIFI_AP);
  WiFi.setSleep(false);
  esp_wifi_set_ps(WIFI_PS_NONE);

  IPAddress apIP(192, 168, 4, 1);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));

  WiFi.softAP(WIFI_SSID, WIFI_PASS, channel);
}

static void setup_station() {
  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false);
  esp_wifi_set_ps(WIFI_PS_NONE);

  WiFi.begin(WIFI_SSID, WIFI_PASS);

  // wait for connection (timeout ~10s)
  int attempts = 0;
  const int maxAttempts = 20;
  while (WiFi.status() != WL_CONNECTED && attempts < maxAttempts) {
    delay(500);
    attempts++;
  }
}

void WIFIC_init(bool is_station) {
  if (is_station) {
    setup_station();
  } else {
    setup_ap();
  }
}
