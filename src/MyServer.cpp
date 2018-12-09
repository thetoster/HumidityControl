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
#include "misc/Prefs.h"
#include "Updater.h"
#include <sha256.h>

const String versionString = "2.0.0";

static const char rootHtml[] PROGMEM =
  #include "www/index.html"
;

static const char setupHtml[] PROGMEM =
    #include "www/setup.html"
;

static const char netConfigHtml[] PROGMEM =
    #include "www/netConfig.html"
;

ESP8266WebServer httpServer(80);
MyServer myServer;
static const char* www_realm = "Authentication Failed";

namespace {

bool checkAuth() {
  if (not httpServer.authenticate(prefs.storage.username,
      prefs.storage.userPassword)) {
    httpServer.requestAuthentication(DIGEST_AUTH, www_realm);
    return false;
  };
  return true;
}

bool checkExtAuth() {
  if ((not httpServer.hasHeader("nonce")) or
      (not httpServer.hasHeader("HMac"))) {
    return checkAuth();
  };
  String nonce = httpServer.header("nonce");
  Sha256.initHmac(prefs.storage.securityKey, sizeof(prefs.storage.securityKey));
  Sha256.print(nonce);
  for(int t = 0; t < httpServer.args(); t++) {
    Sha256.print(httpServer.argName(t));
    Sha256.print(httpServer.arg(t));
  }
  Sha256.print(nonce);

  String hmac;
  hmac.reserve(64);
  uint8_t* hash = Sha256.resultHmac();
  for (int t = 0; t < 32; t++) {
    hmac.concat( (char)(65 + (hash[t]>>4)));
    hmac.concat( (char)(65 + (hash[t]&0xF)));
  }
  Serial.print("nonce:");
  Serial.println(nonce);
  Serial.print("   my hmac:");
  Serial.println(hmac);
  Serial.print("other hmac:");
  Serial.println(httpServer.header("HMac"));
  return hmac.compareTo(httpServer.header("HMac")) == 0;
}

void handleNotFound(){
  if (checkAuth() == false) {
    return;
  }
  httpServer.send(404, "text/plain", "404: Not found");
}

void handleClearHistory() {
  if (checkAuth() == false) {
    return;
  }
  envLogic.measurements.clear();
  httpServer.send(200, "text/plain", "200: OK");
}

void handleFactoryConfig() {
  if (checkAuth() == false) {
    return;
  }
  prefs.defaultValues();
  prefs.save();
  httpServer.send(200, "text/plain", "200: OK");
  myServer.switchToConfigMode();
}

void handleVersion() {
  if (checkAuth() == false) {
    return;
  }
  StaticJsonBuffer<50>  jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  root["version"] = versionString;
  String response;
  root.printTo(response);
  httpServer.send(200, "application/json", response);
}

String toHexString(uint8_t* data, int len) {
  String result;
  for (int t = 0; t < len; t++) {
    uint8_t b = (data[t] >> 4) & 0x0F;
    char c = (b < 0xA) ? '0' + b : 'A' + b - 0xA;
    result.concat(c);

    b = data[t] & 0x0F;
    c = (b < 0xA) ? '0' + b : 'A' + b - 0xA;
    result.concat(c);
  }

  return result;
}

void handleNetConfig() {
  if (checkAuth() == false) {
    return;
  }
  String html = FPSTR(netConfigHtml);

  //network
  html.replace("${ssid}", prefs.storage.ssid);
  html.replace("${inNetworkName}", prefs.storage.inNetworkName);
  html.replace("${username}", prefs.storage.username);
  html.replace("${secureKey}", toHexString(prefs.storage.securityKey,
      sizeof(prefs.storage.securityKey)));

  httpServer.send(200, "text/html", html);
}

void handleGetConfig() {
  if (checkAuth() == false) {
    return;
  }
  StaticJsonBuffer<400>  jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();

  //Network
  root["ssid"] = prefs.storage.ssid;
  root["inNetworkName"] = prefs.storage.inNetworkName;

  //fan
  root["muteFanOn"] = prefs.storage.muteFanOn;
  root["muteFanOff"] = prefs.storage.muteFanOff;

  //heuristic
  root["selectedHeuristic"] = prefs.storage.selectedHeuristic;
  root["useDisturber"] = prefs.storage.useDisturber;
  root["disturberTriggerTime"] = prefs.storage.disturberTriggerTime;
  root["noSamples"] = prefs.storage.noSamples;
  root["timeToForget"] = prefs.storage.timeToForget;
  root["knownHumDiffTrigger"] = prefs.storage.knownHumDiffTrigger;
  root["humidityTrigger"] = prefs.storage.humidityTrigger;

  String response;
  root.printTo(response);
  httpServer.send(200, "application/json", response);
}

String getStringArg(String argName, int maxLen, bool* isError) {
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

int getIntArg(String argName, int maxValue, bool* isError) {
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

bool emplaceChars(char* ptr, String argName, int maxLen) {
  bool fail;
  String tmp = getStringArg(argName, maxLen, &fail);
  if (not fail) {
    strncpy(ptr, tmp.c_str(), maxLen);
  }
  return fail;
}


bool handleSingleHex(const char& c, uint8_t& val) {
  if (c >= '0' && c <= '9') {
    val += c - '0';
  } else if (c >= 'A' && c <= 'F') {
    val += 10 + c - 'A';
  } else if (c >= 'a' && c <= 'f') {
    val += 10 + c - 'a';
  } else {
    return false;
  }

  return true;
}

int hexStringToArray(String hex, uint8_t* data, size_t mLen){
  size_t hexLen = hex.length() / 2;
  if ((hex.length() % 2 != 0) or (hexLen > mLen)) {
    memset(data, 0, mLen);
    return 0;
  }

  int index = 0;
  for (auto it = hex.begin(); it != hex.end();) {
    uint8_t val = 0;

    bool success = handleSingleHex(*it, val);
    it++;
    val <<= 4;

    success &= handleSingleHex(*it, val);
    it++;

    if (not success) {
      memset(data, 0, mLen);
      return 0;
    }

    data[index] = val;
    index++;
  }
  return mLen;
}

bool emplaceHex(uint8_t* ptr, String argName, int maxLen) {
  bool fail;
  String tmp = getStringArg(argName, maxLen * 2 + 2, &fail);
  if (not fail) {
    hexStringToArray(tmp, ptr, maxLen);
  }
  return fail;
}

bool handleNetworkConfig(SavedPrefs& p) {
  bool fail = false;

  fail |= emplaceChars(p.ssid, "ssid", sizeof(p.ssid));
  fail |= emplaceChars(p.password, "wifiPassword", sizeof(p.password));
  fail |= emplaceChars(p.inNetworkName, "inNetworkName", sizeof(p.inNetworkName));
  fail |= emplaceChars(p.username, "username", sizeof(p.username));
  fail |= emplaceChars(p.userPassword, "userPassword", sizeof(p.userPassword));
  fail |= emplaceHex(p.securityKey, "securityKey", sizeof(p.securityKey));

  return fail;
}

bool handleFanConfig(SavedPrefs& p) {
  bool fail = true;

  p.muteFanOn = getIntArg("muteFanOn", 255, &fail);
  if (not fail) {
    p.muteFanOff = getIntArg("muteFanOff", 255, &fail);
  }

  return fail;
}

bool handleHeuristicConfig(SavedPrefs& p) {
  bool fail = true;

  p.humidityTrigger = getIntArg("humidityTrigger", 100, &fail);
  if (not fail) {
    p.selectedHeuristic = getIntArg("selectedHeuristic", 255, &fail);
  }
  if (not fail) {
    p.useDisturber = getIntArg("useDisturber", 255, &fail) != 0 ? true : false;
  }
  if (not fail) {
    p.disturberTriggerTime = getIntArg("disturberTriggerTime", 65535, &fail);
  }
  if (not fail) {
    p.noSamples = getIntArg("noSamples", 120, &fail);
  }
  if (not fail) {
    p.timeToForget = getIntArg("timeToForget", 65535, &fail);
  }
  if (not fail) {
    p.knownHumDiffTrigger = getIntArg("knownHumDiffTrigger", 100, &fail);
  }

  return fail;
}

bool ismemzero(uint8_t* ptr, size_t size) {
  for(size_t t = 0; t < size; t++) {
    if (*ptr != 0) {
      return false;
    }
    ptr++;
  }
  return true;
}

void applyNetConfig(SavedPrefs& p, bool& changed, bool& doNetRestart) {

  if ((strncmp(prefs.storage.inNetworkName, p.inNetworkName, sizeof(p.inNetworkName)) != 0)
      && (strnlen(p.inNetworkName, sizeof(p.inNetworkName)) > 0)) {
    strncpy(prefs.storage.inNetworkName, p.inNetworkName, sizeof(prefs.storage.inNetworkName));
    doNetRestart = true;
    changed = true;
  }
  if ((strncmp(prefs.storage.password, p.password, sizeof(prefs.storage.password)) != 0)
      && (strnlen(p.password, sizeof(p.password)) > 0)) {
    strncpy(prefs.storage.password, p.password, sizeof(prefs.storage.password));
    doNetRestart = true;
    changed = true;
  }
  if ((strncmp(prefs.storage.ssid, p.ssid, sizeof(prefs.storage.ssid)) != 0)
      && (strnlen(p.ssid, sizeof(p.ssid)) > 0)) {
    strncpy(prefs.storage.ssid, p.ssid, sizeof(prefs.storage.ssid));
    doNetRestart = true;
    changed = true;
  }
  if ((strncmp(prefs.storage.username, p.username, sizeof(prefs.storage.username)) != 0)
      && (strnlen(p.username, sizeof(p.username)) > 0)) {
    strncpy(prefs.storage.username, p.username, sizeof(prefs.storage.username));
    changed = true;
  }
  if ((strncmp(prefs.storage.userPassword, p.userPassword, sizeof(prefs.storage.userPassword)) != 0)
      && (strnlen(p.userPassword, sizeof(p.userPassword)) > 0)) {
    strncpy(prefs.storage.userPassword, p.userPassword, sizeof(prefs.storage.userPassword));
    changed = true;
  }
  if ((memcmp(prefs.storage.securityKey, p.securityKey, sizeof(p.securityKey)) != 0) and
      (not ismemzero(p.securityKey, sizeof(p.securityKey))) ) {
    memcpy(prefs.storage.securityKey, p.securityKey, sizeof(p.securityKey));
    changed = true;
  }
}

template<typename T>
void applyIfChanged(T& from, T& to, bool& changed) {
  if (from != to) {
    to = from;
    changed = true;
  }
}

void applyFanConfig(SavedPrefs& p, bool& changed) {
  applyIfChanged(p.muteFanOn, prefs.storage.muteFanOn, changed);
  applyIfChanged(p.muteFanOff, prefs.storage.muteFanOff, changed);
}

void applyHeuristicConfig(SavedPrefs& p, bool& changed) {
  applyIfChanged(p.humidityTrigger, prefs.storage.humidityTrigger, changed);
  applyIfChanged(p.selectedHeuristic, prefs.storage.selectedHeuristic, changed);
  applyIfChanged(p.useDisturber, prefs.storage.useDisturber, changed);
  applyIfChanged(p.disturberTriggerTime, prefs.storage.disturberTriggerTime, changed);
  applyIfChanged(p.noSamples, prefs.storage.noSamples, changed);
  applyIfChanged(p.timeToForget, prefs.storage.timeToForget, changed);
  applyIfChanged(p.knownHumDiffTrigger, prefs.storage.knownHumDiffTrigger, changed);
}

bool applyPrefsChange(SavedPrefs& p, bool& restartNetwork) {
  bool changed = false;
  applyNetConfig(p, changed, restartNetwork);
  applyFanConfig(p, changed);
  applyHeuristicConfig(p, changed);

  return changed | restartNetwork;
}

void handleSetConfig() {
  if (checkAuth() == false) {
    return;
  }

  SavedPrefs p = {0};

  bool fail = handleNetworkConfig(p);
  fail |= handleFanConfig(p);
  fail |= handleHeuristicConfig(p);

  if (fail) {
    return;
  }

  //now apply new values
  bool restartNetwork = false;
  bool changed = applyPrefsChange(p, restartNetwork);

  String result = "200: OK";
  if (changed) {
    prefs.save();
    result += ", Config Saved";
  }

  delay(200);
  if (restartNetwork) {
    result += ", Network restarted";
  }
  httpServer.send(200, "text/plain", result);
  if (restartNetwork) {
    myServer.restart();
  }
}

void handleRun() {
  Serial.println("handleRun");
  if (checkExtAuth() == false) {
    return;
  }
  bool fail = false;
  int sec = getIntArg("time", 40 * 60, &fail);
  Serial.print("Sec:");
  Serial.print(sec);
  Serial.print(", fail:");
  Serial.println(fail);
  if (fail == false && sec > 0) {
      envLogic.requestRunFor(sec);
      httpServer.send(200, "text/plain", "200: OK");
      Serial.println("OK");
  } else {
    httpServer.send(400, "text/plain", "400: BAD REQUEST");
  }
}

void getHistoryData(int count, String& labelData, String& humData) {
  labelData = "";
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
    humData.concat( String(m->humidity) );
    humData.concat(",");
  }
  labelData.remove(labelData.length() - 1);
  humData.remove(labelData.length() - 1);
}

void handleRoot() {
  if (checkAuth() == false) {
    return;
  }
  //put config inside
  String html = FPSTR(rootHtml);
  String labels, hums;
  getHistoryData(60, labels, hums);
  html.replace("${dataLabels}", labels);
  html.replace("${dataHum}", hums);
  httpServer.send(200, "text/html", html);
  delay(100);
  httpServer.client().stop();
}

void handleSetup() {
  if (checkAuth() == false) {
    return;
  }
  String html = FPSTR(setupHtml);

  //fan
  html.replace("${muteFanOff}", String(prefs.storage.muteFanOff));
  html.replace("${muteFanOn}", String(prefs.storage.muteFanOn));

  //heuristic
  html.replace("${humidityTrigger}", String(prefs.storage.humidityTrigger));
  html.replace("${disturberTriggerTime}", String(prefs.storage.disturberTriggerTime));
  html.replace("${noSamples}", String(prefs.storage.noSamples));
  html.replace("${timeToForget}", String(prefs.storage.timeToForget));
  html.replace("${knownHumDiffTrigger}", String(prefs.storage.knownHumDiffTrigger));
  //checkbox values
  html.replace("${useDisturber_defVal}", prefs.storage.useDisturber != 0 ? "checked":" ");
  String heur("heur");
  heur += prefs.storage.selectedHeuristic;
  html.replace("${selectedHeuristic}", heur);

  httpServer.send(200, "text/html", html);
  delay(100);
  httpServer.client().stop();
}

void handleUpdate() {
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
  delay(100);
  httpServer.client().stop();
}

void handleHistory() {
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
    item["D"] = iter->timestamp;
    items.add(item);
  }
  String response;
  root.printTo(response);
  httpServer.send(200, "application/json", response);
  delay(100);
  httpServer.client().stop();
}

void handleStatus() {
  if (checkExtAuth() == false) {
    return;
  }
  StaticJsonBuffer<50>  jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  if (httpServer.hasArg("Simplified")) {
    root["l1"] = "Wilgotnosc:";
    root["l2"] = String(envLogic.getHumidity()) + " %";

  } else {
    root["H"] = envLogic.getHumidity();
    root["D"] = millis();
  }
  String response;
  root.printTo(response);
  httpServer.send(200, "application/json", response);
  delay(100);
  httpServer.client().stop();
}

}

MyServer::MyServer() : needsConfig(true) {
  httpServer.on("/", handleRoot);
  httpServer.on("/netSetup", HTTP_GET, handleNetConfig);
  httpServer.on("/config", HTTP_GET, handleGetConfig);
  httpServer.on("/factoryReset", handleFactoryConfig);
  httpServer.on("/config", HTTP_POST, handleSetConfig);
  httpServer.on("/status", handleStatus);
  httpServer.on("/history", handleHistory);
  httpServer.on("/run", HTTP_POST, handleRun);
  httpServer.on("/clearHistory", handleClearHistory);
  httpServer.on("/update", HTTP_POST, handleUpdate);
  httpServer.on("/version", HTTP_GET, handleVersion);
  httpServer.on("/setup", HTTP_GET, handleSetup);
  httpServer.onNotFound(handleNotFound);

  const char * headerkeys[] = {"nonce", "HMac"} ;
  size_t headerkeyssize = sizeof(headerkeys) / sizeof(char*);
  //ask server to track these headers
  httpServer.collectHeaders(headerkeys, headerkeyssize);
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
  memset(prefs.storage.userPassword, 0, sizeof(prefs.storage.userPassword));
  memset(prefs.storage.password, 0, sizeof(prefs.storage.password));
  for(int t = 0; t < 8; t++) {
    int r = ESP8266TrueRandom.random(10);
    if (r < 3) {
      prefs.storage.userPassword[t] = ESP8266TrueRandom.random('Z'-'A') + 'A';

    } else if (r < 6) {
      prefs.storage.userPassword[t] = ESP8266TrueRandom.random('9'-'0') + '0';

    } else {
      prefs.storage.userPassword[t] = ESP8266TrueRandom.random('z'-'a') + 'a';
    }
  }
  memcpy(prefs.storage.password, prefs.storage.userPassword, 8);
  for(size_t t = 0; t < sizeof(prefs.storage.securityKey); t++) {
    prefs.storage.securityKey[t] = ESP8266TrueRandom.random(256);
  }
}

String MyServer::getServerIp() {
  return needsConfig ? WiFi.softAPIP().toString() : WiFi.localIP().toString();
}

bool MyServer::isServerConfigured() {
  return needsConfig == false;
}

String MyServer::getPassword() {
  return prefs.storage.userPassword[0] == 0 ? "<-->" :
      String(prefs.storage.userPassword);
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

  httpServer.begin();
  MDNS.notifyAPChange();
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
