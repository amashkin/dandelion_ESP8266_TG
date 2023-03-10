#ifndef __DANLOGIC_H_
#define __DANLOGIC_H_

#include "Adafruit_HTU21DF.h"

#define PIN_D5 14  // ESP8266 D5 is GPIO14. Fan switch
#define PIN_D6 12  // ESP8266 D6 is GPIO12. Water pump switch

int iHumidity = 0;
int iTemperature = 0;

const long lInterval = 60000;              // Updates HT readings every 60 seconds
const long lSwitch01_interval = 600000;    // Updates Switch 01 interval = 30 min
const long lSwitch01_run = 30000;          // Switch 01 switched on 30 sec
const long lSwitch02_interval = 28800000;  // Switch 02 work interval 8 h
const long lSwitch02_run = 25000;          // Switch 02 switched on 25 sec

unsigned long previousHMillis = 0;         // will store last time H was updated
unsigned long previousTMillis = 0;         // will store last time T was updated
unsigned long previousMillisSwitch01 = 0;
unsigned long previousMillisSwitch02 = 0;

String getReadableTime(unsigned long lMillis);

Adafruit_HTU21DF htSensor = Adafruit_HTU21DF();  // Humidity\Tempearature Sencor 

void danLogicSetup() {
  pinMode(PIN_D5, OUTPUT);    // Initialise digital pin 14 as an output pin
  pinMode(PIN_D6, OUTPUT);    // Initialise digital pin 12 as an output pin

  digitalWrite(PIN_D5, LOW);  // Switch off in case of issues before restart
  digitalWrite(PIN_D6, LOW);
  digitalWrite(16, HIGH);
   
  if (!htSensor.begin()) {
    Serial.println("Couldn't find sensor!");
  }
}

void htSensorHandle() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousTMillis >= lInterval) {

    previousTMillis = currentMillis;  // save the last time you updated the T value
    
    float newT = htSensor.readTemperature();
    delay(5);
    if (isnan(newT)) {  // if temperature read failed, don't change t value
      Serial.println("Failed to read Temperature from HT sensor!");
    } else {
      iTemperature = newT;

      Serial.print("Temerature: ");      
      Serial.println(newT);
    }
  }

  if (currentMillis - previousHMillis >= lInterval) {

    previousHMillis = currentMillis;  // save the last time you updated the T value

    float newH = htSensor.readHumidity();
    delay(5);
    if (isnan(newH)) {  // if humidity read failed, don't change h value
      Serial.println("Failed to read humidity from HT sensor!");
    } else {
      iHumidity = newH;
      Serial.print("Humidity: ");
      Serial.println(newH);
    }
  }  
}

// Swithch ON Fan at D5 every lSwitch01_interval for lSwitch01_run seconds
void switch01Handle() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillisSwitch01 >= lSwitch01_interval) {
    previousMillisSwitch01 = currentMillis;
    Serial.println("Switch 01 On");
    digitalWrite(PIN_D5, HIGH);
   
    delay(lSwitch01_run);
    digitalWrite(PIN_D5, LOW);
    Serial.println("Switch 01 Off");
  
  }
}

void runPumpOnce() {
  Serial.println("Water pump. Switch 02 On");
  digitalWrite(PIN_D6, HIGH);
 
  int iMills = millis();
  delay(lSwitch02_run);
  digitalWrite(PIN_D6, LOW);
  Serial.println(millis()-iMills);
  Serial.println("Water pump. Switch 02 Off");

}

// Swithch on D6 for lSwitch02_run seconds
// Run water pump periodicaly
void switch02Handle() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillisSwitch02 >= lSwitch02_interval) {
    previousMillisSwitch02 = currentMillis;
    runPumpOnce();
  }
}

String getTelemetry() {
  unsigned long currentMillis = millis();
  String sToNextSwitch01 = getReadableTime( currentMillis - previousMillisSwitch01 );
  String sToNextSwitch02 = getReadableTime( currentMillis - previousMillisSwitch02 );
  String sStatus = "Fan: " + sToNextSwitch01 + " WPump: " + sToNextSwitch02 + " Hum: " + iHumidity + " Tem: " + iTemperature;
  return sStatus;
}

void danLogicHandle() {
  htSensorHandle();  // HT Sensor 
  switch01Handle();  // Fan switch handle. Job to run Fan every X=10 min for Y=30 sec
  switch02Handle();  // Water pump switch handle. Job to run every Z=8h for S=25 sec
}

bool bSwitch01 = LOW;
bool bSwitch02 = LOW;
void runSwitch01() {
  Serial.println("Fan. Switch 01 changed");
  bSwitch01 = !bSwitch01;
  Serial.println(bSwitch01);
  digitalWrite(PIN_D5, bSwitch01);
}

void runSwitch02() {
  Serial.println("Fan. Switch 02 changed");
  runPumpOnce(); // It runs once to avoid issues with overload. Just to keep this action on the AndroidCloud
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
    sReadableTime = String(days) + " ";
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