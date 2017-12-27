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

 Prefs.cpp
 Created on: Dec 27, 2017
 Author: Bartłomiej Żarnowski (Toster)
 */
#include <Prefs.h>
#include <EEPROM.h>

Prefs::Prefs() : ssid(""), password(""), humidityTrigger(60) {
  char buff[30]; //max ssid, max password
  EEPROM.begin(512);
  EEPROM.get(0, buff);
  ssid = buff;
  EEPROM.get(0 + sizeof(buff), buff);
  password = buff;
  EEPROM.get(0 + sizeof(buff) + sizeof(buff), humidityTrigger);
  EEPROM.end();
}

void Prefs::save() {
  char buff[30];
  EEPROM.begin(512);
  memset(&buff[0], 0, 30);
  strncpy(&buff[0], ssid.c_str(), 30);
  EEPROM.put(0, buff);

  memset(&buff[0], 0, 30);
  strncpy(&buff[0], password.c_str(), 30);
  EEPROM.put(0 + sizeof(buff), buff);

  EEPROM.put(0 + sizeof(buff) + sizeof(buff), humidityTrigger);
  EEPROM.commit();
  EEPROM.end();
}
