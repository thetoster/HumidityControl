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

MyServer::MyServer(Prefs* prefs) : prefs(prefs) {
  if (prefs->storage.password[0] == 0) {
    generateRandomPassword();

  } else {
    needsConfig = false;
  }

}

void MyServer::generateRandomPassword() {
  needsConfig = true;
  for(int t = 0; t < 8; t++) {
    int r = ESP8266TrueRandom.random(10);
    if (r < 3) {
      prefs->storage.password[t] = ESP8266TrueRandom.random('Z'-'A') + 'A';

    } else if (r < 6) {
      prefs->storage.password[t] = ESP8266TrueRandom.random('9'-'0') + '0';

    } else {
      prefs->storage.password[t] = ESP8266TrueRandom.random('z'-'a') + 'a';
    }
  }
}

String MyServer::getServerIp() {
  return "192.168.0.4"; //todo: Not sure if always :P
}

bool MyServer::isServerConfigured() {
  return needsConfig;
}

String MyServer::getPassword() {
  return String(prefs->storage.password);
}
