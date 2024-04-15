#include "M5StickCPlus2.h"
#include <ArduinoMqttClient.h>
#include "WiFi.h"
//MQTT variables
const char broker[] = "mqtt.ugavel.com";
int mqtt_port = 1883;
WiFiClient wifiClient;
MqttClient mqttClient(wifiClient);
const char topic1[] = "ugaelee2045sp24/kjohnsen/topic1";

void setup() {
  pinMode(4, OUTPUT); // Set HOLD pin 04 as output
  digitalWrite(4, HIGH);  //IMPORTANT, Set HOLD pin to high to maintain power supply or M5StickCP2 will turn off
  auto cfg = M5.config();
  StickCP2.begin(cfg);
  //connectWifi(); // runs a captive portal (very opinionated)
  connectToEnterpriseWifi(); // note, must modify SimpleWifi.ino
  //connectToPersonalWifi(); 
  StickCP2.Display.clear(); //clear stuff from wifi connect

  //MQTT setup
  mqttClient.onMessage(onMqttMessage);
  mqttClient.setUsernamePassword("class_user", "class_password");
  mqttClient.connect(broker, mqtt_port);
  mqttClient.subscribe(topic1);

  Serial.println("Setup finished");

}

void loop() {
  mqttClient.poll(); // Checks for MQTT messages
  StickCP2.update(); // Updates button states
  if(StickCP2.BtnA.wasPressed()){
    // Publish the raw bytes over MQTT
    mqttClient.beginMessage(topic1);
    mqttClient.print("Hello from M5Stick");
    mqttClient.endMessage();
  }
}

void onMqttMessage(int messageSize) {
  Serial.print("Received a message from topic '");
  Serial.print(mqttClient.messageTopic());
  Serial.print("', length ");
  Serial.print(messageSize);
  Serial.println(" bytes:");
  if (mqttClient.messageTopic() == topic1) {
      Serial.println(mqttClient.readString()); //note, binary is more efficient for MQTT communication
  }
}