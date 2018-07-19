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

 Prefs.h
 Created on: Dec 27, 2017
 Author: Bartłomiej Żarnowski (Toster)
 */
#ifndef Prefs_hpp
#define Prefs_hpp
#include <Arduino.h>

struct SavedPrefs {
    uint8_t crc;

    //Used by net manager
    char ssid[60];
    char password[60];
    char inNetworkName[20];

    //Used by Fan
    uint8_t muteFanOn;	//request to turn fan on will be ignored for this long after fan turn off
    uint8_t muteFanOff; //request to turn fan off will be ignored for this long after fan turn on (minimal run time)

    //Heuristics data
    uint8_t selectedHeuristic;
    uint8_t useDisturber;	//0..1 as bool, used by AdaptiveHeuristic1/2 and NiceToHaveHeuristic
    uint16_t disturberTriggerTime;	//in seconds, period on which disturber will run
    uint8_t noSamples;	//used by AdaptiveHeuristic1/2
    uint16_t timeToForget; //in seconds, used by NiceToHaveHeuristic
    uint8_t knownHumDiffTrigger; //in % used by NiceToHaveHeuristic
    int8_t humidityTrigger;	//used by LimiterHeuristic and Disturber
};

class Prefs {
  public:
    SavedPrefs storage;

    void save();
    void defaultValues();
    bool hasPrefs();
    void load();
  private:
    uint8_t calcCRC();

    bool isZeroPrefs();
};

extern Prefs prefs;
#endif /* Prefs_hpp */
