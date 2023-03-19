#ifndef __DANLOGIC_H_
#define __DANLOGIC_H_

#define SWITCH_01 14 // ESP8266 D5 is GPIO14. Fan switch
#define SWITCH_02 12 // ESP8266 D6 is GPIO12. Water pump switch

#include <ArduinoOTA.h>
#include "Adafruit_HTU21DF.h"
#include "ESP8266_TG.h"

const long lInterval = 60000;              // Updates HT readings every 60 seconds
const long lSwitch01_interval = 600000;    // Updates Switch 01 interval = 30 min
const long lSwitch01_run = 30000;          // Switch 01 switched on 30 sec
const long lSwitch02_interval = 28800000;  // Switch 02 work interval 8 h
const long lSwitch02_run = 20000;          // Switch 02 switched on 20 sec

unsigned long previousMillis = 0;          // will store last time T was updated
unsigned long previousMillisSwitch01 = 0;
unsigned long previousMillisSwitch02 = 0;

void danLogicSetup(UniversalTelegramBot *b);
String getReadableTime(unsigned long lMillis);
float getHumidity();
float getTemperature();
float getSoilMisture();
void switchRun(int iPin, unsigned long lRunMillis);
void danLogicHandle();
String getTelemetry();
void requestConfig();

Adafruit_HTU21DF htSensor = Adafruit_HTU21DF();  // Humidity\Tempearature Sencor 
extern UniversalTelegramBot *getBot();

void danLogicSetup() {
  pinMode(SWITCH_01, OUTPUT);    // Initialise digital pin 14 as an output pin
  pinMode(SWITCH_02, OUTPUT);    // Initialise digital pin 12 as an output pin
 
  if (!htSensor.begin()) {
    Serial.println("danLogicSetup(): Couldn't find sensor!");
  }
  requestConfig();
}

float getHumidity() {
    Serial.println("getHumidity()");
    float h = htSensor.readHumidity();
    delay(5);
    if (isnan(h)) {  
      Serial.println("getHumidity():Failed to read Humidity from HT sensor!");
      return -1;
    } else {
      Serial.print("Humidity:  "+ (String)h);
      return h;
    }
}

float getTemperature() {
    Serial.println("getTemperature()");
    float t = htSensor.readTemperature();
    delay(5);
    if (isnan(t)) {  
      Serial.println("getHumidity():Failed to read Temperature from HT sensor!");
      return -1;
    } else {
      Serial.print("Humidity:  "+ (String)t);
      return t;
    }
}

float getSoilMisture() {
  Serial.println("getSoilMisture()");
    
  analogWriteFreq(75000);
  analogWrite(A0, 512);
  // read the value from the sensor:
  float f = analogRead(A0);
  analogWrite(A0, 0);
  Serial.println("Soil moisture: " + (String)f);

  return f;
}

void switchRun(int iPin, unsigned long lRun) {
  Serial.println("switchRun()");
  Serial.println("switchRun():Switch " + (String)iPin + " On");
  digitalWrite(iPin, HIGH);
  delay(lRun);    //should be changed
  digitalWrite(iPin, LOW);
  Serial.println("switchRun():Switch " + (String)iPin + " Off");
}

void danLogicHandle() {
  unsigned long now = millis();
  if (now - previousMillis >= lInterval) {
    previousMillis = now;                                      
    float t =  getTemperature();
    float h =  getHumidity();
    float sm = getSoilMisture();
    // sendTelemetry();    
  }

  if (now - previousMillisSwitch01 >= lSwitch01_interval) {
    previousMillisSwitch01 = now;    
    switchRun(SWITCH_01, lSwitch01_run);  
  }

  if (now - previousMillisSwitch02 >= lSwitch02_interval) {
    previousMillisSwitch02 = now;
    switchRun(SWITCH_02, lSwitch02_run);
    UniversalTelegramBot *pBot = getBot();
    if(pBot != NULL) {  
      pBot->sendMessage(OM_TG_ID, "Pump runs.", "Markdown");
      String sMsg = getTelemetry();
      pBot->sendMessage(OM_TG_ID, sMsg, "Markdown");
    } else 
    Serial.println("Internal Error: void danLogicHandle()");
  }
}

String getTelemetry() {
  unsigned long now = millis();
  String sHN   = ArduinoOTA.getHostname();
  String sNow = getReadableTime(now);
  String s01  = getReadableTime(now - previousMillisSwitch01);
  String s02  = getReadableTime(now - previousMillisSwitch02);
  String sF   = getReadableTime(lSwitch01_interval);
  String sP   = getReadableTime(lSwitch02_interval);
  float h  = getHumidity();
  float t  = getTemperature();
  float sm = getSoilMisture();

  String sRet = "*DeviceID*: " + sHN + "\n*Restarted:* " + sNow + "\n*Fan*: " + s01 + "->" + sF + "\n*Water Pump*: " + s02 +"->"+ sP +"\n*Humidity*: " + h + "\n*Temperature*: " + t + " \n*Soil moisture*: "+ sm;
  Serial.println("Telemetry:\n" + sRet);
  return sRet;
}

void requestConfig() {
  UniversalTelegramBot *pBot = getBot();
  if(pBot != NULL) {  
    String sMsg = "*DeviceID*: " + ArduinoOTA.getHostname() + " started. Request Configuration."; 
    pBot->sendMessage(OM_TG_ID, sMsg, "Markdown");
  } else 
    Serial.println("Internal Error: void danLogicHandle()");
}

String getReadableTime(unsigned long lMillis) {
  String sReadableTime;
  unsigned long seconds;
  unsigned long minutes;
  unsigned long hours;
  unsigned long days;

  seconds = lMillis / 1000;
  minutes = seconds / 60;
  hours = minutes / 60;
  days = hours / 24;
  lMillis %= 1000;
  seconds %= 60;
  minutes %= 60;
  hours %= 24;

  if (days > 0) {
    sReadableTime = String(days) + ":";
  }

  if (hours > 0) {
    sReadableTime += String(hours) + ":";
  }

  if (minutes < 10) {
    sReadableTime += "0";
  }
  sReadableTime += String(minutes) + ":";

  if (seconds < 10) {
    sReadableTime += "0";
  }
  sReadableTime += String(seconds);
  
  return sReadableTime;
}

#endif