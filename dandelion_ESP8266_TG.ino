// Board: Generic ESP8266 Module

#include "ESP8266_TG.h"
void setup() {
  Serial.begin(115200);
  Serial.println();
  ESP8266_TG_setup();
}

void loop() {
  ESP8266_TG_loop();
}
