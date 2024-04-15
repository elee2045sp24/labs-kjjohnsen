#include <M5StickCPlus2.h>
#include <BLEDevice.h>

void setup(){
  StickCP2.begin();
  BLEDevice::init("M5StickCPlus-Kyle");
  BLEDevice::startAdvertising();
}

void loop(){

}

