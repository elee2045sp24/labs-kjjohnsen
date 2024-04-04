#include "M5StickCPlus2.h"
#include <ArduinoMqttClient.h>
#include "WiFi.h"
//MQTT variables
const char broker[] = "mqtt.ugavel.com";
int mqtt_port = 1883;
WiFiClient wifiClient;
MqttClient mqttClient(wifiClient);

const char acc_topic[] = "elee2045sp24/controller/kjohnsen/acc";
const char but_topic[] = "elee2045sp24/controller/kjohnsen/but";

void setup() {
  pinMode(4, OUTPUT); // Set HOLD pin 04 as output
  digitalWrite(4, HIGH);  //IMPORTANT, Set HOLD pin to high to maintain power supply or M5StickCP2 will turn off
  auto cfg = M5.config();
  StickCP2.begin(cfg);
  connectToAnyWifi(); //new function to try to try connect to both types of wifi
  //MQTT setup
  mqttClient.setUsernamePassword("class_user", "class_password");
  mqttClient.connect(broker, mqtt_port);
  Serial.println("Setup finished");
}
void send_but(int down){
  mqttClient.beginMessage(but_topic);
  char buffer[100];
  sprintf((char*)buffer,"%d",down);
  mqttClient.print(buffer);
  mqttClient.endMessage();
}
void send_acc(){
  auto data = StickCP2.Imu.getImuData();
  mqttClient.beginMessage(acc_topic);
  char buffer[100];
  sprintf((char*)buffer,"%f,%f,%f",data.accel.x,data.accel.y,data.accel.z);
  mqttClient.print(buffer);
  mqttClient.endMessage();
}
void loop() {
  mqttClient.poll(); // Checks for MQTT messages
  StickCP2.update(); // Updates button states
  
  if(StickCP2.BtnA.wasPressed()){
    send_but(1);
  }
  if(StickCP2.BtnA.wasReleased()){
    send_but(0);
  }

  auto imu_update = StickCP2.Imu.update();
  if(imu_update){
    send_acc();
  }
}