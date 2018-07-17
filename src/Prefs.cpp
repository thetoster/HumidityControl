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

Prefs prefs;

Prefs::Prefs() {
/*
  defaultValues();

  EEPROM.begin(512);
  EEPROM.get(0, storage);
  EEPROM.end();

  if (storage.crc != calcCRC()) {
    defaultValues();
  }
*/
}

void Prefs::load() {
  EEPROM.begin(512);
  EEPROM.get(0, storage);
  EEPROM.end();
  uint8_t expectedCrc = calcCRC();

  Serial.print("StorageCrc:");
  Serial.println(storage.crc);
  Serial.print("Expected CRC:");
  Serial.println(expectedCrc);
  Serial.print("Is zero prefs:");
  Serial.println(isZeroPrefs());
  Serial.flush();

  //if (storage.crc != expectedCrc || isZeroPrefs()) {
  if (not hasPrefs()) {
    defaultValues();
  }
}

bool Prefs::hasPrefs() {
  return (storage.crc == calcCRC()) && (not isZeroPrefs()) && (prefs.storage.ssid[0] != 0);
}

bool Prefs::isZeroPrefs() {
  const uint8_t* data = (uint8_t*)&storage;
  for(size_t t = 0; t < sizeof(storage); t++) {
    if (*data != 0) {
      return false;
    }
    data++;
  }
  return true;
}

void Prefs::defaultValues() {
  Serial.println("Reset prefs to default");
  storage.humidityTrigger = 60;
  storage.secondsToStoreMeasurements = 60;
  memset(&storage.ssid[0], 0, sizeof(storage.ssid));
  memset(&storage.password[0], 0, sizeof(storage.password));
  memset(&storage.inNetworkName[0], 0, sizeof(storage.inNetworkName));
  strcpy(&storage.inNetworkName[0], "HumSensor");
}

void Prefs::save() {
  storage.crc = calcCRC();
  EEPROM.begin(512);
  EEPROM.put(0, storage);
  EEPROM.commit();
  EEPROM.end();
}

uint8_t Prefs::calcCRC() {
  const uint8_t* data = (uint8_t*)&storage;
  data++; //skip crc field
  int len = sizeof(storage) - 1;
  uint8_t crc = 0x00;
  while (len--) {
    byte extract = *data++;
    for (byte tempI = 8; tempI; tempI--) {
      byte sum = (crc ^ extract) & 0x01;
      crc >>= 1;
      if (sum) {
        crc ^= 0x8C;
      }
      extract >>= 1;
    }
  }
  return crc;
}
