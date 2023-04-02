#ifndef __ESP8266_TG_
#define __ESP8266_TG_

#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>

#include "keepAss.h"
#include "danLogic.h"

//
bool isUserInList(String sUser);
//

const unsigned long BOT_MTBS = 2000; // mean time between scan messages
const String sCmd = "\n\n*/fan*, /pump, */telemetry*, \n*/config*, /pumpPlus, /pumpMinus, /pumpRunPlus, /pumpRunMinus";

unsigned long bot_lasttime; // last time messages' scan has been done
X509List cert(TELEGRAM_CERTIFICATE_ROOT);
WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOT_TOKEN, secured_client);
UniversalTelegramBot *getBot() {
  return &bot;
}


void handleNewMessages(int numNewMessages) {
  Serial.print("handleNewMessages ");
  Serial.println(numNewMessages);
  
  String answer;
  for (int i = 0; i < numNewMessages; i++)   {
    telegramMessage &msg = bot.messages[i];
    Serial.println("Received command " + msg.text);
    Serial.println("Received fromID " + msg.from_id);
    
    if (msg.text == "/help")
      answer = "So you need _help_, uh? me too! use /start or /status";
    else if (msg.text == "/start")
      answer = "Guten morgen my friend *" + msg.from_name + "*. Find ` to execute - send /help command";
    else if (msg.text == "/status")
      answer = "All is good here, thanks for asking!";
    else if (msg.text == "/fan" && isUserInList(msg.from_id))
      switchRun(SWITCH_01, lSwitch01_run);
    else if (msg.text == "/pump" && isUserInList(msg.from_id))
      switchRun(SWITCH_02, lSwitch02_run);
    else if (msg.text == "/telemetry")
      answer = getTelemetry() + "\n\n/config, /telemetry";
    else if (msg.text == "/pumpPlus") {
      setPumpIntervalPlus();
      answer = getConfig()+ "\n\n/config, /telemetry, /pumpMinus, */pumpPlus*";
    } 
    else if (msg.text == "/pumpMinus") {
      setPumpIntervalMinus();
      answer = getConfig() + "\n\n/config, /telemetry, */pumpMinus*, /pumpPlus";
    }
    else if (msg.text == "/pumpRunPlus") {
      setPumpRunIntervalPlus();
      answer = getConfig()+ "\n\n/config, /telemetry, /pumpRunMinus, */pumpRunPlus*";
    }
    else if (msg.text == "/pumpRunMinus") {
      setPumpRunIntervalMinus();
      answer = getConfig()+ "\n\n/config, /telemetry, */pumpRunMinus*, /pumpRunPlus";
    }

    else if (msg.text == "/config")
      answer = getConfig() + "\n\n/pumpMinus, /pumpPlus, /pumpRunMinus, /pumpRunPlus" ;
    else
      answer = "Do what?";

    bot.sendMessage(msg.chat_id, answer, "Markdown");
  
    if(msg.text == "/help" and isUserInList(msg.from_id)) {
      bot.sendMessage(msg.chat_id, sCmd, "Markdown");      
    }   
  }
}

//check security
bool isUserInList(String sUser) {
  if (sUser == OM_TG_ID or sUser == OM_TG_ID) 
    return true;
  return false;
}

void bot_setupCommands() {
  const String commands = F("["
                            "{\"command\":\"help\",  \"description\":\"Get bot usage help\"},"
                            "{\"command\":\"start\", \"description\":\"Message sent when you open a chat with a bot\"},"
                            "{\"command\":\"status\",\"description\":\"Answer device current status\"}" // no comma on last command
                            "]");
  bot.setMyCommands(commands);
}

void ESP8266_TG_setup() {
  Serial.begin(115200);
  Serial.println();

  // attempt to connect to Wifi network:
  configTime(0, 0, "pool.ntp.org");      // get UTC time via NTP
  secured_client.setTrustAnchors(&cert); // Add root certificate for api.telegram.org
  Serial.print("Connecting to Wifi SSID ");
  Serial.print(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.print("\nWiFi connected. IP address: ");
  Serial.println(WiFi.localIP());
  bot_setupCommands();
}

void ESP8266_TG_loop() {
  if (millis() - bot_lasttime > BOT_MTBS) {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while (numNewMessages) {
      Serial.println("got response");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }

    bot_lasttime = millis();
  }
}
#endif