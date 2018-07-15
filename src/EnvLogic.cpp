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

 EnvLogic.cpp
 Created on: Dec 27, 2017
 Author: Bartłomiej Żarnowski (Toster)
 */
#include <EnvLogic.h>
#include "Prefs.h"

static constexpr int totalMeasurementMemoryLimit = 10 * 1024;
static constexpr long MIN_AUTO_TIME = 10 * 1000;

EnvLogic envLogic;

EnvLogic::EnvLogic() :
  lastTemp(0), lastHum(0), requestedRunToMillis(0),
  turnOnFanMillis(0), lastMeasurementMillis(0), lastUpdate(0), autoFanOnMillis(0),
  lastMotorState(true) {
  fanMotor(false);
}

void EnvLogic::requestRunFor(int seconds) {
  requestedRunToMillis = millis() + seconds * 1000;
  fanMotor(true);
}

int EnvLogic::getMaxAllowedHum() {
  return prefs.storage.humidityTrigger;
}

void EnvLogic::fanMotor(bool enabled) {
  if (lastMotorState != enabled) {
    Serial.println(enabled ? "Fan:ON" : "Fan:OFF");
    digitalWrite(FAN_CONTROL_PIN, enabled ? HIGH : LOW);
    lastMotorState = enabled;
  }
}

void EnvLogic::update() {
  bool wasFanOn = isFanEnabled();

  if (millis() - lastUpdate > 1000) {
    lastTemp = sht.getTemperature();
    lastHum = sht.getHumidity();
    lastUpdate = millis();

    if (lastHum >= getMaxAllowedHum()) {
      autoFanOnMillis =  MIN_AUTO_TIME;

    } else {
      autoFanOnMillis --;
    }
  }

  if (wasFanOn == false && isFanEnabled()) {
    turnOnFanMillis = millis();
    fanMotor(true);

  } else if (isFanEnabled() == false) {
    fanMotor(false);
  }

  //store new measurement
  long mil = millis();
  if (mil - lastMeasurementMillis > prefs.storage.secondsToStoreMeasurements * 1000) {
    addMeasurement(mil);
  }
}

void EnvLogic::addMeasurement(long mil) {
  lastMeasurementMillis = mil;
  int totalSize = sizeof(Measurement) * measurements.size() + 1;
  if (totalSize >= totalMeasurementMemoryLimit) {
    measurements.erase(measurements.begin());
  }
  measurements.push_back(Measurement(mil, lastHum, lastTemp));
}

bool EnvLogic::isFanEnabled() {
  return (autoFanOnMillis > 0) || (millis() < (unsigned long)requestedRunToMillis);
}

String EnvLogic::getDisplayTemp() {
  char buf[10];
  memset(&buf[0], 0, 8);

  itoa(lastTemp, &buf[0], 10);
  String temp = "Temp.:";
  for(int t = 0; buf[t] != 0; t++) {
    temp += buf[t];
  }
  temp += "C";
  return temp;
}

String EnvLogic::getDisplayHum() {
  char buf[10];
  memset(&buf[0], 0, 8);
  itoa(lastHum, &buf[0], 10);
  String hum = "Wilg.:";
  for(int t = 0; buf[t] != 0; t++) {
    hum += buf[t];
  }
  hum += "%";
  return hum;
}

String EnvLogic::getDisplayFan() {
  long fanTime;
  if (millis() < (unsigned long)requestedRunToMillis) {
    fanTime = requestedRunToMillis - millis();

  } else {
    fanTime = millis() - turnOnFanMillis;
  }
  return "Nawiew " + millisToTime(fanTime);
}

String millisToTime(long mil) {
  String text;
  mil /= 1000;
  if (mil < 60) {
    text += mil;
    text += "s";

  } else if (mil < 3600){
    text += mil / 60;
    text += ":";
    if (mil % 60 < 10) {
      text += "0";
    }
    text += mil % 60;
  }
  return text;
}
