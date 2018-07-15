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
#include "Updater.h"

const String versionString = "1.1.0";
const static String rootHtml =
    #include "www/index.html"
;

ESP8266WebServer httpServer(80);
MyServer myServer;
static const char* www_username = "Lampster";
static const char* www_realm = "Authentication Failed, use login name:Lampster";

static bool checkAuth() {
  if (httpServer.authenticate(www_username, prefs.storage.password) == false) {
    httpServer.requestAuthentication(DIGEST_AUTH, www_realm);
    return false;
  };
  return true;
}

static void handleNotFound(){
  if (checkAuth() == false) {
    return;
  }
  httpServer.send(404, "text/plain", "404: Not found");
}

static void handleClearHistory() {
  if (checkAuth() == false) {
    return;
  }
  envLogic.measurements.clear();
  httpServer.send(200, "text/plain", "200: OK");
}

static void handleFactoryConfig() {
  if (checkAuth() == false) {
    return;
  }
  prefs.defaultValues();
  prefs.save();
  httpServer.send(200, "text/plain", "200: OK");
  myServer.switchToConfigMode();
}

static void handleVersion() {
  if (checkAuth() == false) {
    return;
  }
  DynamicJsonBuffer  jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  root["version"] = versionString;
  String response;
  root.printTo(response);
  httpServer.send(200, "application/json", response);
}

static void handleGetConfig() {
  if (checkAuth() == false) {
    return;
  }
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

static String getStringArg(String argName, int maxLen, bool* isError) {
  String result = "";
  *isError = false;
  if (httpServer.hasArg(argName)) {
    result = httpServer.arg(argName);
    if (result.length() >= (unsigned int)maxLen) {
      String resp = "406: Not Acceptable, '" + argName + "' to long.";
      httpServer.send(406, "text/plain", resp);
      *isError = true;
    }
  }
  return result;
}

static int getIntArg(String argName, int maxValue, bool* isError) {
  int result = -1;
  *isError = false;
  if (httpServer.hasArg(argName)) {
    result = httpServer.arg(argName).toInt();
    if (result >= maxValue) {
      String resp = "406: Not Acceptable, '" + argName + "' to big.";
      httpServer.send(406, "text/plain", resp);
      *isError = true;
    }
  }
  return result;
}

static void handleSetConfig() {
  if (checkAuth() == false) {
    return;
  }
  bool fail;
  String ssid = getStringArg("ssid", sizeof(prefs.storage.ssid), &fail);
  if (fail == true) {
    return;
  }

  String pass = getStringArg("password", sizeof(prefs.storage.password), &fail);
  if (fail == true) {
    return;
  }

  String name = getStringArg("inNetName", sizeof(prefs.storage.inNetworkName), &fail);
  if (fail == true) {
    return;
  }

  int humTrig = getIntArg("humTrigger", 100, &fail);
  if (fail == true) {
    return;
  }

  int histInt = getIntArg("addHistoryInterval", 255, &fail);
  if (fail == true) {
    return;
  }

  //now apply new values
  bool changed = false;
  bool restartNetwork = false;
  if (humTrig > 0) {
    prefs.storage.humidityTrigger = humTrig;
    changed = true;
  }
  if (histInt > 0) {
    prefs.storage.secondsToStoreMeasurements = histInt;
    changed = true;
  }
  if (name.length() > 0) {
    strncpy(prefs.storage.inNetworkName, name.c_str(), sizeof(prefs.storage.inNetworkName));
    changed = true;
    restartNetwork = true;
  }
  if (pass.length() > 0) {
    strncpy(prefs.storage.password, pass.c_str(), sizeof(prefs.storage.password));
    changed = true;
    restartNetwork = true;
  }
  if (ssid.length() > 0) {
    strncpy(prefs.storage.ssid, ssid.c_str(), sizeof(prefs.storage.ssid));
    changed = true;
    restartNetwork = true;
  }
  String result = "200: OK";
  if (changed) {
    prefs.save();
    result += ", Config Saved";
  }
  //httpServer.send(200, "text/plain", "200: OK");
  delay(200);
  if (restartNetwork) {
    result += ", Network restarted";
  }
  httpServer.send(200, "text/plain", result);
  if (restartNetwork) {
    myServer.restart();
  }
}

static void handleRun() {
  if (checkAuth() == false) {
    return;
  }
  bool fail = false;
  int sec = getIntArg("time", 40 * 60, &fail);
  if (fail == false && sec > 0) {
      envLogic.requestRunFor(sec);
      httpServer.send(200, "text/plain", "200: OK");
  } else {
    httpServer.send(400, "text/plain", "400: BAD REQUEST");
  }
}

static void getHistoryData(int count, String& labelData, String &tempData, String& humData) {
  labelData = "";
  tempData = "";
  humData = "";
  auto m = envLogic.measurements.begin();
  if ((unsigned int )count < envLogic.measurements.size()) {
    m += envLogic.measurements.size() - count;
  }
  long mil = millis();
  for(; m != envLogic.measurements.end(); m++) {
    labelData.concat("'");
    labelData.concat( millisToTime(mil - m->timestamp) );
    labelData.concat("',");
    tempData.concat( String(m->temp) );
    tempData.concat(",");
    humData.concat( String(m->humidity) );
    humData.concat(",");
  }
  labelData.remove(labelData.length() - 1);
  tempData.remove(labelData.length() - 1);
  humData.remove(labelData.length() - 1);
}

static void handleRoot() {
  if (checkAuth() == false) {
    return;
  }
  //put config inside
  String html = rootHtml;
  html.replace("${ssid}", prefs.storage.ssid);
  html.replace("${inNetName}", prefs.storage.inNetworkName);
  html.replace("${humTrigger}", String(prefs.storage.humidityTrigger));
  html.replace("${addHistoryInterval}", String(prefs.storage.secondsToStoreMeasurements));
  String labels, temps, hums;
  getHistoryData(60, labels, temps, hums);
  html.replace("${dataLabels}", labels);
  html.replace("${dataTemp}", temps);
  html.replace("${dataHum}", hums);
  httpServer.send(200, "text/html", html);
}

static void handleUpdate() {
  if (checkAuth() == false) {
    return;
  }
  bool fail = false;
  String url = getStringArg("url", 1024, &fail);
  if (fail == false && url.length() > 0) {
      httpServer.send(200, "text/plain", "200: OK");
      updater.execute(url);

  } else {
    httpServer.send(400, "text/plain", "400: BAD REQUEST");
  }
}

static void handleHistory() {
  if (checkAuth() == false) {
    return;
  }
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
  if (checkAuth() == false) {
    return;
  }
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
//  restart();
}

void MyServer::switchToConfigMode() {
  WiFi.setAutoReconnect(false);
  WiFi.disconnect(false);
  WiFi.enableAP(false);
  WiFi.enableSTA(false);
  delay(500);
  memset(prefs.storage.ssid, 0, sizeof(prefs.storage.ssid));
  generateRandomPassword();
  needsConfig = true;
  enableSoftAP();
}

void MyServer::connectToAccessPoint() {
  WiFi.softAPdisconnect(false);
  WiFi.begin(prefs.storage.ssid, prefs.storage.password);
  WiFi.setAutoReconnect(true);
  WiFi.setAutoConnect(true);
}

void MyServer::generateRandomPassword() {
  memset(prefs.storage.password, 0, sizeof(prefs.storage.password));
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
  return needsConfig == false;
}

String MyServer::getPassword() {
  return prefs.storage.password[0] == 0 ? "<-->" : String(prefs.storage.password);
}

void MyServer::enableSoftAP() {
  WiFi.softAP(prefs.storage.inNetworkName, prefs.storage.password);
}

void MyServer::restart() {
  httpServer.stop();
  WiFi.softAPdisconnect(false);
  WiFi.setAutoReconnect(false);
  WiFi.setAutoConnect(false);
  WiFi.disconnect(false);
  WiFi.enableAP(false);
  WiFi.enableSTA(false);

  needsConfig = not prefs.hasPrefs();
  if (needsConfig) {
    generateRandomPassword();
    enableSoftAP();

  } else {
    connectToAccessPoint();
  }
  httpServer.on("/", handleRoot);
  httpServer.on("/config", HTTP_GET, handleGetConfig);
  httpServer.on("/factoryReset", handleFactoryConfig);
  httpServer.on("/config", HTTP_POST, handleSetConfig);
  httpServer.on("/status", handleStatus);
  httpServer.on("/history", handleHistory);
  httpServer.on("/run", HTTP_POST, handleRun);
  httpServer.on("/clearHistory", handleClearHistory);
  httpServer.on("/update", HTTP_POST, handleUpdate);
  httpServer.on("/version", HTTP_GET, handleVersion);
  httpServer.onNotFound(handleNotFound);

  httpServer.begin();
  MDNS.begin(prefs.storage.inNetworkName);
  MDNS.addService("http", "tcp", 80);
}

String MyServer::getStatus() {
  switch(WiFi.status()) {
    case WL_CONNECTED:
      return "";
    case WL_DISCONNECTED:
      return "Odlaczony";
    case WL_IDLE_STATUS:
      return "Bezczynny";
    case WL_NO_SSID_AVAIL:
      return "Brak SSID";
    case WL_SCAN_COMPLETED:
      return "Zeskanowane";
    case WL_CONNECT_FAILED:
      return "Blad polaczenia";
    case WL_CONNECTION_LOST:
      return "Utracono pol.";
    default:
      return "?";
  }
}

void MyServer::update() {
  MDNS.update();
  httpServer.handleClient();
}
