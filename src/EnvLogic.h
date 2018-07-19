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

 EnvLogic.h
 Created on: Dec 27, 2017
 Author: Bartłomiej Żarnowski (Toster)
 */
#ifndef EnvLogic_hpp
#define EnvLogic_hpp

#include "Measurement.h"
#include "Fan.h"
#include "Heuristic.h"
#include <SHT21.h>
#include <vector>

class EnvLogic {
  public:
    int lastTemp;
    int lastHum;
    std::vector<Measurement> measurements;

    EnvLogic();
    void update();
    String getDisplayTemp();
    String getDisplayHum();
    String getDisplayFan();
    bool isFanRunning();
    void requestRunFor(int seconds);
  private:
    const uint8_t FAN_CONTROL_PIN = 12;
    const uint8_t UNUSED_CTRL_PIN = 13;
    SHT21 sht;
    Fan fan{FAN_CONTROL_PIN};
    long requestedRunToMillis;
    long lastUpdate;
    std::vector<Heuristic*> heuristics;

    int getMaxAllowedHum();
    void addMeasurement(long mil);

    bool isTooWet();
    bool fanIsRequested();
    void collectMeasurementIfNeeded();
    Heuristic* getSelectedHeuristic();
};

extern EnvLogic envLogic;
String millisToTime(long mil);

#endif /* EnvLogic_hpp */
