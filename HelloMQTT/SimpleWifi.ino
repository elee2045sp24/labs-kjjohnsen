#include "esp_wpa2.h"  //wpa2 library for connections to Enterprise networks
#include "WiFi.h"
#include "WifiSecrets.h"
void connectToEnterpriseWifi() {
  WiFi.disconnect(true);  //disconnect form wifi to set new wifi connection
  WiFi.mode(WIFI_STA);    //init wifi mode
  esp_wifi_sta_wpa2_ent_set_identity((uint8_t *)"", 0);
  esp_wifi_sta_wpa2_ent_set_username((uint8_t *)WIFI_USERNAME, strlen(WIFI_USERNAME));
  esp_wifi_sta_wpa2_ent_set_password((uint8_t *)WIFI_PASSWORD, strlen(WIFI_PASSWORD));
  esp_wifi_sta_wpa2_ent_enable();
  WiFi.begin(WIFI_SSID);  //connect to wifi
  waitForWifi();
}
void connectToPersonalWifi() {
  WiFi.disconnect(true);
  WiFi.mode(WIFI_STA);  //init wifi mode
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  waitForWifi();
}

void waitForWifi() {
  WiFi.setSleep(false);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Waiting for connection");
  }
  IPAddress ip = WiFi.localIP();
  Serial.println(ip);
}