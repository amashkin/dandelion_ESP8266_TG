// Board: Generic ESP8266 Module
#include "danOTA.h"     //Define OTA before ESP32_cam
#include "ESP8266_TG.h"
#include "danLogic.h"

void setup() {
  Serial.begin(115200);

  OTA_init();                                   // danOTA.h Over The Air
  ESP8266_TG_setup(); // Telegram Events logic
  danLogicSetup();                             // Data\Action logic
}

void loop() {
  ArduinoOTA.handle();
  ESP8266_TG_loop();
  danLogicHandle();
}