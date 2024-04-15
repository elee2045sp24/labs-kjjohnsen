#include "esp_wpa2.h"  //wpa2 library for connections to Enterprise networks
#include "WiFi.h"
#include "WifiSecrets.h"
bool connectToEnterpriseWifi() {
  WiFi.disconnect(true);  //disconnect form wifi to set new wifi connection
  WiFi.mode(WIFI_STA);    //init wifi mode
  esp_wifi_sta_wpa2_ent_set_identity((uint8_t *)"", 0);
  esp_wifi_sta_wpa2_ent_set_username((uint8_t *)ENTERPRISE_WIFI_USERNAME, strlen(ENTERPRISE_WIFI_USERNAME));
  esp_wifi_sta_wpa2_ent_set_password((uint8_t *)ENTERPRISE_WIFI_PASSWORD, strlen(ENTERPRISE_WIFI_PASSWORD));
  esp_wifi_sta_wpa2_ent_enable();
  WiFi.begin(ENTERPRISE_WIFI_SSID);  //connect to wifi
  return waitForWifi();
}
bool connectToPersonalWifi() {
  WiFi.disconnect(true);
  WiFi.mode(WIFI_STA);  //init wifi mode
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  return waitForWifi();
}

void connectToAnyWifi(){
  while(true){
    if(connectToEnterpriseWifi()){
      break;
    }else if(connectToPersonalWifi()){
      break;
    }
  }
}

bool waitForWifi() {
  int numTries = 0;
  WiFi.setSleep(false);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    numTries++;
    if(numTries > 10){
      return false;
    }
    Serial.println("Waiting for connection");
  }
  IPAddress ip = WiFi.localIP();
  Serial.println(ip);
  return true;
}