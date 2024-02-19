#include "M5StickCPlus2.h"
#include <ArduinoMqttClient.h>
#include "WiFi.h"
//MQTT variables
const char broker[] = "mqtt.ugavel.com";
int mqtt_port = 1883;
WiFiClient wifiClient;
MqttClient mqttClient(wifiClient);

const char battery_topic[] = "elee2045sp24/doorbell/kjohnsen/battery";
const char pressed_topic[] = "elee2045sp24/doorbell/kjohnsen/pressed";
const char mode_topic[] = "elee2045sp24/doorbell/kjohnsen/mode";
const char message_topic[] = "elee2045sp24/doorbell/kjohnsen/message";

typedef enum {NORMAL, DND} MODE;
MODE mode = NORMAL;

unsigned long pressTime;
unsigned long lastStatusTime;
bool waitingForRelease=false;

void setup() {
  pinMode(4, OUTPUT); // Set HOLD pin 04 as output
  digitalWrite(4, HIGH);  //IMPORTANT, Set HOLD pin to high to maintain power supply or M5StickCP2 will turn off
  auto cfg = M5.config();
  StickCP2.begin(cfg);
  connectToEnterpriseWifi(); // note, must modify SimpleWifi.ino
  //connectToPersonalWifi(); 
  StickCP2.Display.clear(); //clear stuff from wifi connect

  //MQTT setup
  mqttClient.onMessage(onMqttMessage);
  mqttClient.setUsernamePassword("class_user", "class_password");
  mqttClient.connect(broker, mqtt_port);
  mqttClient.subscribe(mode_topic);
  mqttClient.subscribe(message_topic);
  Serial.println("Setup finished");

  lastStatusTime = millis();
}

void send_ring(){
  mqttClient.beginMessage(pressed_topic);
  mqttClient.print("pressed");
  mqttClient.endMessage();
}
void loop() {
  mqttClient.poll(); // Checks for MQTT messages
  StickCP2.update(); // Updates button states
  unsigned long currentTime = millis();
  if(mode==NORMAL){
    if(StickCP2.BtnA.wasPressed()){
      send_ring();
    }
    
  }
  else if(mode==DND){
    if(StickCP2.BtnA.wasPressed()){
      pressTime = millis();
      waitingForRelease = true;
    }
    if(currentTime-pressTime > 3000){
      if(waitingForRelease){
        send_ring();
      }
      waitingForRelease = false;
    }

  }

  if(currentTime-lastStatusTime > 1000){
    lastStatusTime = currentTime;
    mqttClient.beginMessage(battery_topic);
    mqttClient.printf("%d",StickCP2.Power.getBatteryLevel());
    mqttClient.endMessage();
  }
}

void onMqttMessage(int messageSize) {
  Serial.print("Received a message from topic '");
  Serial.print(mqttClient.messageTopic());
  Serial.print("', length ");
  Serial.print(messageSize);
  Serial.println(" bytes:");
  if (mqttClient.messageTopic() == message_topic) {
      String message = mqttClient.readString();
      StickCP2.Lcd.clear();
      StickCP2.Lcd.setCursor(0,0);
      StickCP2.Lcd.print(message.c_str());
      StickCP2.Speaker.tone(2000,1000);
      
  }else if (mqttClient.messageTopic() == mode_topic) {
      String message = mqttClient.readString();
      if(message == "Normal"){
        mode = NORMAL;
      }else if(message == "DND"){
        mode = DND;
      }
      Serial.println(message);
  }
}