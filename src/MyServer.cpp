/*
 BSD 3-Clause License

 Copyright (c) 2017, The Tosters
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright notice, this
 list of conditions and the following disclaimer.

 * Redistributions in binary form must reproduce the above copyright notice,
 this list of conditions and the following disclaimer in the documentation
 and/or other materials provided with the distribution.

 * Neither the name of the copyright holder nor the names of its
 contributors may be used to endorse or promote products derived from
 this software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 WifiServer.cpp
 Created on: Dec 29, 2017
 Author: Bartłomiej Żarnowski (Toster)
 */
#include <ESP8266TrueRandom.h>
#include <MyServer.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>
#include "EnvLogic.h"
#include "Prefs.h"

ESP8266WebServer httpServer(80);
MyServer myServer;

static void handleNotFound(){
  httpServer.send(404, "text/plain", "404: Not found");
}

static void handleGetConfig() {
  DynamicJsonBuffer  jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  root["ssid"] = prefs.storage.ssid;
  root["humTrigger"] = prefs.storage.humidityTrigger;
  root["inNetName"] = prefs.storage.inNetworkName;
  root["addHistoryInterval"] = prefs.storage.secondsToStoreMeasurements;
  String response;
  root.printTo(response);
  httpServer.send(200, "application/json", response);
}

static void handleSetConfig() {

}

static void handleHistory() {
  DynamicJsonBuffer  jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  JsonArray& items = root.createNestedArray("items");
  root["now"] = millis();
  for(auto iter = envLogic.measurements.begin(); iter != envLogic.measurements.end(); iter++) {
    JsonObject& item = jsonBuffer.createObject();
    item["H"] = iter->humidity;
    item["T"] = iter->temp;
    item["D"] = iter->timestamp;
    items.add(item);
  }
  String response;
  root.printTo(response);
  httpServer.send(200, "application/json", response);
}

static void handleStatus() {
  DynamicJsonBuffer  jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  root["H"] = envLogic.lastHum;
  root["T"] = envLogic.lastTemp;
  root["D"] = millis();
  String response;
  root.printTo(response);
  httpServer.send(200, "application/json", response);
}

MyServer::MyServer() {
  if (prefs.storage.password[0] == 0) {
    generateRandomPassword();
    enableSoftAP();

  } else {
    needsConfig = false;
    connectToAccessPoint();
  }
  httpServer.on("/", handleNotFound);
  httpServer.on("/config", HTTP_GET, handleGetConfig);
  httpServer.on("/config", HTTP_POST, handleSetConfig);
  httpServer.on("/status", handleStatus);
  httpServer.on("/history", handleHistory);
  httpServer.onNotFound(handleNotFound);

  httpServer.begin();
}

void MyServer::connectToAccessPoint() {
  WiFi.begin(prefs.storage.ssid, prefs.storage.password);
  MDNS.begin(prefs.storage.inNetworkName);
}

void MyServer::generateRandomPassword() {
  needsConfig = true;
  for(int t = 0; t < 8; t++) {
    int r = ESP8266TrueRandom.random(10);
    if (r < 3) {
      prefs.storage.password[t] = ESP8266TrueRandom.random('Z'-'A') + 'A';

    } else if (r < 6) {
      prefs.storage.password[t] = ESP8266TrueRandom.random('9'-'0') + '0';

    } else {
      prefs.storage.password[t] = ESP8266TrueRandom.random('z'-'a') + 'a';
    }
  }
}

String MyServer::getServerIp() {
  return needsConfig ? WiFi.softAPIP().toString() : WiFi.localIP().toString();
}

bool MyServer::isServerConfigured() {
  return needsConfig;
}

String MyServer::getPassword() {
  return String(prefs.storage.password);
}

void MyServer::enableSoftAP() {
  WiFi.softAP(prefs.storage.inNetworkName, prefs.storage.password);
}
