#include <M5StickCPlus2.h>
#include <driver/i2s.h>
#include <WiFi.h>
#include <ArduinoMqttClient.h>

const char* host = "172.20.71.211"; // Replace with the IP address of the server you want to connect to
int port = 8080; // Replace with the port number you want to connect to

//MQTT variables
const char broker[] = "mqtt.ugavel.com";
int mqtt_port = 1883;
const char mic_button_topic[] = "ugaelee2045sp24/kjohnsen/mic";
const char llama_response_topic[] = "ugaelee2045sp24/kjohnsen/llamaresponse";
WiFiClient tcpClient;
MqttClient mqttClient(tcpClient);

WiFiUDP udpClient;
#define PIN_CLK     0
#define PIN_DATA    34
#define BUFFER_SIZE  512
#define SAMPLE_RATE 48000
uint8_t BUFFER[BUFFER_SIZE] = {0};

void i2sInit() {
    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX | I2S_MODE_PDM),
        .sample_rate = SAMPLE_RATE, //very important 
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,  // 2 bytes for each sample
        .channel_format = I2S_CHANNEL_FMT_ALL_RIGHT, //this seems strange, but if this were a stereo microphone, you could have interleaved L/R data.  This says it's ALL_RIGHT data
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1, 
        .dma_buf_count    = 4, //must have at least 2 buffers (1 getting filled, 1 getting read)
        .dma_buf_len      = BUFFER_SIZE/2,  //note, this is samples, not bytes, so x2 because data is 16 bits
    };

    i2s_pin_config_t pin_config;

    pin_config.mck_io_num = I2S_PIN_NO_CHANGE;
    pin_config.bck_io_num   = I2S_PIN_NO_CHANGE;
    pin_config.ws_io_num    = PIN_CLK;
    pin_config.data_out_num = I2S_PIN_NO_CHANGE;
    pin_config.data_in_num  = PIN_DATA;

    i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
    i2s_set_pin(I2S_NUM_0, &pin_config);
    i2s_set_clk(I2S_NUM_0, SAMPLE_RATE, I2S_BITS_PER_SAMPLE_16BIT, I2S_CHANNEL_MONO);
}

void onMqttMessage(int messageSize) {
  Serial.print("Received a message from topic '");
  Serial.print(mqttClient.messageTopic());
  Serial.print("', length ");
  Serial.print(messageSize);
  Serial.println(" bytes:");
  if (mqttClient.messageTopic() == llama_response_topic) {
      
      M5.Lcd.fillScreen(WHITE);
      M5.Lcd.setTextColor(BLACK, WHITE);
      M5.Lcd.setCursor(0,0);
      M5.Lcd.println(mqttClient.readString());
  }
}
void setup() {
    M5.begin();
    connectToAnyWifi(); //new function to try to try connect to both types of wifi
    
    M5.Lcd.setRotation(3);
    M5.Lcd.fillScreen(WHITE);
    M5.Lcd.setTextColor(BLACK, WHITE);
    M5.Lcd.println("mic test");

    //MQTT setup
    mqttClient.onMessage(onMqttMessage);
    mqttClient.setUsernamePassword("class_user", "class_password");
    mqttClient.connect(broker, mqtt_port);
    mqttClient.subscribe(llama_response_topic);
    i2sInit();
}

void loop() {
  mqttClient.poll(); // Checks for MQTT messages
  StickCP2.update();
  size_t bytesread;
  i2s_read(I2S_NUM_0, (char *)BUFFER, BUFFER_SIZE, &bytesread, 1000); 
  udpClient.beginPacket(host,port);
  udpClient.write(BUFFER,bytesread);
  udpClient.endPacket();

  if(StickCP2.BtnA.wasPressed()){
    // Publish the raw bytes over MQTT
    mqttClient.beginMessage(mic_button_topic);
    uint8_t message[1] = {1};
    mqttClient.write(message,1);
    mqttClient.endMessage();
  }

  if(StickCP2.BtnA.wasReleased()){
    // Publish the raw bytes over MQTT
    mqttClient.beginMessage(mic_button_topic);
    uint8_t message[1] = {0};
    mqttClient.write(message,1);
    mqttClient.endMessage();
  }

}