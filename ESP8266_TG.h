#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>

#include "keepAss.h"
#include "danLogic.h"

//
bool isUserInList(String sUser);
//

const unsigned long BOT_MTBS = 1000; // mean time between scan messages

unsigned long bot_lasttime; // last time messages' scan has been done
X509List cert(TELEGRAM_CERTIFICATE_ROOT);
WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOT_TOKEN, secured_client);

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
      answer = "Guten morgen my friend *" + msg.from_name + "*. Find what commands are available for you - send /help command";
    else if (msg.text == "/status")
      answer = "All is good here, thanks for asking!";
    else if (msg.text == "/fan" && msg.from_id == OM_TG_ID)
      runSwitch01();
    else if (msg.text == "/telemetry")
      answer = getTelemetry();
    else   
      answer = "Do what?";

    bot.sendMessage(msg.chat_id, answer, "Markdown");

    ////////////////////////
    if(msg.text == "/help" and isUserInList(msg.from_id)) {
      bot.sendMessage(msg.chat_id, "You are in the list and some additional commands are *working for you*. /fan, /pump, /telemetry", "Markdown");
      bot.sendMessage(msg.chat_id, "/fan - sends command to run Fan. Run twice to get the same state. On/Off. It is just *to demonstate that it works*", "Markdown");
      bot.sendMessage(msg.chat_id, "/pump - sends command to run Water Pump. *You are not an Admin*. No access to send command to water pump. It demostrates next level of access to the system.  ", "Markdown");
      bot.sendMessage(msg.chat_id, "/telemetry - sends command to request telemetry and other available data. _Humidity_ and _Temperature_ sensors are disconnected for a while.", "Markdown");
    }   
    
    if(msg.text == "/pump" and  isUserInList(msg.from_id)) {
      bot.sendMessage(msg.chat_id, "You do not have access to send command /pump. Use /help to check command's list.", "Markdown");
    }

    if(msg.from_id == OT_TG_ID)
      bot.sendMessage(OT_TG_ID, "Recived command: " + msg.text, "Markdown");
    ////////////////////////
  }
}

//check security
bool isUserInList(String sUser) {
  if (sUser == OM_TG_ID or sUser == OT_TG_ID) 
    return true;
  return false;
}



void bot_setup() {
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

  // Check NTP/Time, usually it is instantaneous and you can delete the code below.
  Serial.print("Retrieving time: ");
  time_t now = time(nullptr);
  while (now < 24 * 3600) {
    Serial.print(".");
    delay(100);
    now = time(nullptr);
  }
  Serial.println(now);

  bot_setup();
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