// Board: Generic ESP8266 Module
#include "danOTA.h"     //Define OTA before ESP32_cam
#include "ESP8266_TG.h"

void setup() {
  Serial.begin(115200);
  Serial.println();
  ESP8266_TG_setup();
  OTA_init();         // danOTA.h Over The Air
  Serial.println("Board: Generic ESP8266 Module");
}

void loop() {
  ESP8266_TG_loop();
  ArduinoOTA.handle();
}