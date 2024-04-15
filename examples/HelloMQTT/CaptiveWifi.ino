//this code handles configuring the wifi.  Just call doWifiSetup to start the portal, and connectToWifi to make the connection
//the only thing it assumes is a Preferences variable named preferences
#include "esp_wpa2.h"  //wpa2 library for connections to Enterprise networks
#include <WebServer.h>
#include <Preferences.h>
#include <DNSServer.h>
WebServer server(80);
const byte DNS_PORT = 53;
DNSServer dnsServer;
String station_mac = "none";
Preferences preferences; //we need this for WiFiParts.ino (could be there as well, but it's generally useful)
void tryWifiSetup(){
  preferences.begin("my-app", false); 
  StickCP2.Display.setRotation(1);
  StickCP2.Display.setTextColor(GREEN);
  StickCP2.Display.setTextDatum(top_center);
  StickCP2.Display.setCursor(0, 20, 2);
  StickCP2.Display.clear();
  StickCP2.Display.drawString("Hold Button for WiFi config", StickCP2.Display.width() / 2, StickCP2.Display.height() / 2);
  delay(2000);  // Two second config delay
  if (digitalRead(37) == LOW) {  // Reads button A
    doWifiWifiSetup();
  }
}
//this synchronous function will try to connect to a wifi network using stored preferences.  It should work with most 2.4ghz networks.
//it displays its progress on the LCD
bool connectWifi() {
  tryWifiSetup(); // will prompt for holding button to try wifi setup (blocks if held)
  station_mac = WiFi.macAddress();
  Serial.println(station_mac);
  String username = preferences.getString("username", "");
  String password = preferences.getString("password", "");
  String ssid = preferences.getString("ssid", "");
  bool is_eap = preferences.getBool("eap", false);

  StickCP2.Display.setCursor(7, 20, 2);
  StickCP2.Display.clear();
  StickCP2.Display.println("Connecting to wifi");
  StickCP2.Display.println(ssid);

  WiFi.mode(WIFI_STA);


  if (is_eap) {
    esp_wifi_sta_wpa2_ent_set_identity((uint8_t *)"", 0);
    esp_wifi_sta_wpa2_ent_set_username((uint8_t *)username.c_str(), username.length());
    esp_wifi_sta_wpa2_ent_set_password((uint8_t *)password.c_str(), password.length());
    esp_wifi_sta_wpa2_ent_enable();

    WiFi.begin(ssid.c_str());  //connect to wifi
  } else {
    WiFi.begin(ssid.c_str(), password.c_str());
  }

  int num_attempts = 0;
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
    num_attempts++;

    if (num_attempts > 60) {
      ESP.restart();
    }
  }

  Serial.println(WiFi.localIP());
  return true;
}

//this function broadcasts a hotspot at the last 4 of the mac address and creates a captive portal that allows people to connect
//it displays progress on the LCD
void doWifiWifiSetup() {

  String randomPassword = generateRandomPassword(8);  // Min of 8 characters needed to set the softAP password
  String macAddress = WiFi.macAddress();
  String softAPssid = macAddress.substring(12);
  delay(100);

  WiFi.mode(WIFI_AP);
  WiFi.softAP(softAPssid.c_str(), randomPassword.c_str());
  Serial.println(randomPassword);

  IPAddress apIP = WiFi.softAPIP();
  StickCP2.Display.println("Wifi setup");
  StickCP2.Display.println("Connect Wifi to");
  StickCP2.Display.println(softAPssid.c_str());
  StickCP2.Display.println("and go to");
  StickCP2.Display.println(apIP);
  StickCP2.Display.println("Password is");
  StickCP2.Display.println(randomPassword);


  dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
  dnsServer.start(DNS_PORT, "*", apIP);

  server.on("/", handle_root);
  server.on("/generate_204", handle_root);
  server.on("/hotspot-detect.html", handle_root);
  server.on("/success.txt", handle_root);
  server.on("/connecttest.txt", handle_root);
  server.on("/wpad.dat", handle_root);
  server.onNotFound(handle_root);
  server.on("/type", handle_wifi);
  server.on("/set", handle_form);
  server.begin();

  while (true) {
    dnsServer.processNextRequest();
    server.handleClient();
  }
}

//helper functions
String toStringIp(IPAddress ip) {
  String res = "";
  for (int i = 0; i < 3; i++) {
    res += String((ip >> (8 * i)) & 0xFF) + ".";
  }
  res += String(((ip >> 8 * 3)) & 0xFF);
  return res;
}

String generateRandomPassword(int length) {
  String charset = "0123456789";
  String password = "";

  for (int i = 0; i < length; i++) {
    int randomIndex = random(charset.length());
    password += charset[randomIndex];
  }

  return password;
}

bool isIp(String str) {
  for (size_t i = 0; i < str.length(); i++) {
    int c = str.charAt(i);
    if (c != '.' && (c < '0' || c > '9')) {
      return false;
    }
  }
  return true;
}

//this function redirects to the captive portal if someone makes any http request not to the root.
//todo, increase font size
bool captivePortal() {
  if (!isIp(server.hostHeader())) {
    Serial.println("Request redirected to captive portal");
    server.sendHeader("Location", String("http://192.168.4.1"), true);
    server.send(302, "text/plain", "");
    server.client().stop();
    return true;
  }
  return false;
}
//this is the main function implementing the captive portal to set wifi information
//here you set the first name and last name and type of network you want to connect to
void handle_root() {
  if (captivePortal()) {
    return;
  }
  String HTML = "<!DOCTYPE html>\
  <html>\
  <body>\
  <form action=\"/type\" method=\"POST\">";
  // Add form fields for first and last name
  HTML += "First Name: <input type='text' name='firstname' autocomplete='off'/><br>";
  HTML += "Last Name: <input type='text' name='lastname' autocomplete='off'/><br>";
  HTML = HTML + "WPA (Home WiFi)<input type=\"radio\" name=\"type\" value=\"wpa\"/><br>\
  EAP (Enterpise / PAWs secure)<input type=\"radio\" name=\"type\" value=\"eap\"/><br>\
  <input type=\"submit\">\
  </body>\
  </html>";
  server.send(200, "text/html", HTML);
}
//this is the second page, handling discovery/selection of wireless networks, usernames and passwords
//todo, update the user on the LCD as to what is going on
void handle_wifi() {
  if (captivePortal()) {
    return;
  }
  String firstname = server.arg("firstname");  // Get the entered first name
  String lastname = server.arg("lastname");    // Get the entered last name
  preferences.putString("firstname", firstname);
  preferences.putString("lastname", lastname);
  String type = server.arg("type");  // Declare 'type' variable

  String HTML = "<!DOCTYPE html>\
  <html>\
  <body>\
  <form action=\"/set\" method=\"POST\">";

  int n = WiFi.scanNetworks();

  // Check if there are any networks available
  if (n > 0) {
    HTML += "Selected Wi-Fi network type: " + type + "<br>";

    for (int i = 0; i < n; i++) {
      String ssid = WiFi.SSID(i);
      bool isEAP = (WiFi.encryptionType(i) == WIFI_AUTH_WPA2_ENTERPRISE);

      // Only display networks that match the selected type
      if ((type == "wpa" && !isEAP) || (type == "eap" && isEAP)) {
        HTML += "<input type='radio' name='ssid' value='" + ssid + "'>" + ssid + "<br>";
      }
    }

    HTML += "password<input type='password' name='password' autocomplete='off'/><br>";

    // Check if the selected type is EAP and include the username field
    if (type == "eap") {
      HTML += "username (UGA ID not 811) <input type='text' name='username' autocomplete='off'/><br>";
    }

    HTML += "<input type='submit'>";
  } else {
    HTML += "No networks found.";
  }

  HTML += "</form></body></html>";
  server.send(200, "text/html", HTML);
}

//this is the final handler that disables the server and connects to the network.
void handle_form() {
  String ssid = server.arg("ssid");
  String password = server.arg("password");
  String username = server.arg("username");
  bool use_eap = false;
  if (username != "") {
    use_eap = true;
  }

  preferences.putString("username", username);
  preferences.putString("ssid", ssid);
  preferences.putString("password", password);
  preferences.putBool("eap", use_eap);

  String HTML = "<!DOCTYPE html>\
  <html>\
  <body>\
  <p>Success.  Disconnecting</p>\
  </body>\
  </html>";

  server.send(200, "text/html", HTML);
  preferences.end();
  ESP.restart();
}