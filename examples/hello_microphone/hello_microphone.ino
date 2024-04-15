#include <M5StickCPlus2.h>
#include <driver/i2s.h>
#include <WiFi.h>

const char* ssid = "shamrock";
const char* password = "3522195419";
const char* host = "192.168.68.131"; // Replace with the IP address of the server you want to connect to
int port = 8080; // Replace with the port number you want to connect to
WiFiUDP client;
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
void setup() {
    M5.begin();
    connectToAnyWifi(); //new function to try to try connect to both types of wifi
    
    M5.Lcd.setRotation(3);
    M5.Lcd.fillScreen(WHITE);
    M5.Lcd.setTextColor(BLACK, WHITE);
    M5.Lcd.println("mic test");

    i2sInit();
}
int framesread = 0;
void loop() {
  size_t bytesread;

  i2s_read(I2S_NUM_0, (char *)BUFFER, BUFFER_SIZE, &bytesread, 1000); 
  client.beginPacket(host,port);
  client.write(BUFFER,bytesread);
  client.endPacket();

}