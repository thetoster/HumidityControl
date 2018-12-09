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
#include "misc/Prefs.h"
#include "heuristic/AdaptiveHeuristic.h"
#include "heuristic/AdaptiveHeuristic2.h"
#include "heuristic/LimiterHeuristic.h"
#include "heuristic/NiceToHaveHeuristic.h"

namespace {
  constexpr float ETA = 0.9;
  constexpr int MEAS_COUNT = 60;
  Measurement measurementBuff[MEAS_COUNT];
}

EnvLogic envLogic;

EnvLogic::EnvLogic() :
    humAverage(0), measurements(
        PreAllocator<Measurement>(measurementBuff, sizeof(MEAS_COUNT))), requestedRunToMillis(0), lastUpdate(0) {

  pinMode(UNUSED_CTRL_PIN, OUTPUT);
  digitalWrite(UNUSED_CTRL_PIN, LOW);
  fan.shouldRun = false;

  heuristics.push_back(new LimiterHeuristic(fan));
  heuristics.push_back(new AdaptiveHeuristic(fan));
  heuristics.push_back(new AdaptiveHeuristic2(fan));
  heuristics.push_back(new NiceToHaveHeuristic(fan));
}

void EnvLogic::requestRunFor(int seconds) {
  requestedRunToMillis = millis() + seconds * 1000;
  fan.shouldRun = true;
}

int EnvLogic::getMaxAllowedHum() {
  return prefs.storage.humidityTrigger;
}

bool EnvLogic::isTooWet() {
  return getHumidity() > getMaxAllowedHum();
}

bool EnvLogic::fanIsRequested() {
  return (millis() < (unsigned long) requestedRunToMillis);
}

int EnvLogic::getHumidity() {
  return static_cast<int>(humAverage);
}

void EnvLogic::update() {
  if (millis() - lastUpdate > 1000) {
    //low-pass filter
    humAverage = (1.0f - ETA) * sht.getHumidity() + ETA * humAverage;
    lastUpdate = millis();
  }

  fan.shouldRun = isTooWet() or fanIsRequested();
  fan.update();

  collectMeasurementIfNeeded();
}

void EnvLogic::collectMeasurementIfNeeded() {
  unsigned long mil = millis();
  if (measurements.size() > 0) {
    const Measurement& mes = measurements.back();
    if (getHumidity() != mes.humidity) {
      addMeasurement(mil);
    }
  } else {
    addMeasurement(mil);
  }
}

void EnvLogic::addMeasurement(unsigned long mil) {
  if (measurements.size() == MEAS_COUNT) {
    measurements.erase(measurements.begin());
  }
  measurements.push_back(Measurement(mil, getHumidity()));
}

bool EnvLogic::isFanRunning() {
  return fan.isRunning();
}

String EnvLogic::getDisplayHum() {
  String hum(getHumidity());
  hum += " %";
  return hum;
}

String EnvLogic::getDisplayFan() {
  long fanTime;
  if (millis() < (unsigned long) requestedRunToMillis) {
    fanTime = requestedRunToMillis - millis();

  } else {
    fanTime = millis() - fan.getTurnOnFanMillis();
  }
  return "Nawiew " + millisToTime(fanTime);
}

String millisToTime(long mil) {
  String text;
  mil /= 1000;
  if (mil < 60) {
    text += mil;
    text += "s";

  } else if (mil < 3600) {
    text += mil / 60;
    text += ":";
    if (mil % 60 < 10) {
      text += "0";
    }
    text += mil % 60;
  }
  return text;
}

Heuristic* EnvLogic::getSelectedHeuristic() {
  int i = prefs.storage.selectedHeuristic;
  if ((i >= (int) heuristics.size()) or (i < 0)) {
    i = 0;
  }
  return heuristics[i];
}
