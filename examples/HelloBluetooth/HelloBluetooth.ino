#include <M5StickCPlus2.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLE2902.h>

#define SERVICE_UUID "82a7e967-7504-4f75-a68e-57c2803d8f40"
#define CHARACTERISTIC_UUID "82a7e967-7504-4f75-a68e-57c2803d8f41"

BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic = NULL;
bool deviceConnected = false;
bool advertising = false;
//int num = 0;
#pragma pack(1)
typedef struct {
  float accx,accy,accz;
  uint16_t batt;
} Packet;

Packet p;

class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* pServer, esp_ble_gatts_cb_param_t* param) {
    Serial.println("Device connected");
    deviceConnected = true;
    advertising = false;
  }
  void onDisconnect(BLEServer* pServer) {
    Serial.println("Device disconnected");
    deviceConnected = false;
  }
};


void setup() {
  StickCP2.begin();
  BLEDevice::init("M5StickCPlus-Kyle");
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  BLEService* pService = pServer->createService(SERVICE_UUID);
  pCharacteristic = pService->createCharacteristic(
    CHARACTERISTIC_UUID,
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);
  pCharacteristic->addDescriptor(new BLE2902());
  pService->start();
  BLEDevice::startAdvertising();
}


void loop() {
  StickCP2.update();
  StickCP2.Imu.update();
  StickCP2.Imu.getAccelData(&p.accx, &p.accy, &p.accz);
  p.batt = StickCP2.Power.getBatteryVoltage();
  if (deviceConnected) {
 
    //pCharacteristic->setValue((uint8_t*)(&num), 2);
    pCharacteristic->setValue((uint8_t*)(&p), sizeof(Packet));
    pCharacteristic->notify();
    //num++;
    delay(10);
  }
  if (!deviceConnected && !advertising) {
    BLEDevice::startAdvertising();
    Serial.println("start advertising");
    advertising = true;
  }
}
