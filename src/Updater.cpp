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

 Updater.cpp
 Created on: Jan 2, 2018
 Author: Bartłomiej Żarnowski (Toster)
 */
#include <Arduino.h>
#include <SSD1306.h>
#include "Updater.h"

extern SSD1306  display;
Updater updater;

Updater::Updater(): delayTimer(0), lastMs(0), screen(UpdaterScreen_NONE) {

}

void Updater::execute(String url) {
  showUpdateInfo();
  ESPhttpUpdate.rebootOnUpdate(true);
  t_httpUpdate_return ret = ESPhttpUpdate.update(url);
  handleUpdateError(ret);
}

void Updater::handleUpdateError(t_httpUpdate_return ret ) {
  switch(ret) {
    case HTTP_UPDATE_FAILED:
      delayTimer = 11;
      screen = UpdaterScreen_FAILED;
      break;

    case HTTP_UPDATE_NO_UPDATES:
      delayTimer = 11;
      screen = UpdaterScreen_NO_UPDATE;
      break;

    default:
    case HTTP_UPDATE_OK:
      //we shouldn't be here!
      break;
  }
}

void Updater::showUpdateInfo() {
  display.clear();
  display.setColor(WHITE);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_16);
  display.drawString(0, 0, "Uaktualnienie");
  display.drawString(0, 16, "Wgrywanie");
  display.drawString(0, 37, "nowej wersji.");
  display.display();
}

String Updater::getDots() {
  String dots;
  dots.reserve(delayTimer);
  for(int t = 0; t < delayTimer; t++) {
    dots += ".";
  }
  return dots;
}

void Updater::showNoUpdateInfo() {
  display.clear();
  display.setColor(WHITE);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_16);
  display.drawString(0, 0, "Uaktualnienie");
  display.drawString(0, 16, "Nic nowego.");
  display.drawString(0, 37, getDots());
  display.display();
}

void Updater::showFailedInfo() {
  display.clear();
  display.setColor(WHITE);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_16);
  display.drawString(0, 0, "Uaktualnienie");
  display.drawString(0, 16, "Nie udalo sie.");
  display.drawString(0, 37, getDots());
  display.display();
}

bool Updater::update() {
  if (delayTimer <= 0) {
    return false;
  }
  if (millis() - lastMs < 1000) {
    delay(250);
    return true;
  }
  lastMs = millis();
  delayTimer--;
  switch(screen) {
  	default:
    case UpdaterScreen_FAILED:
      showFailedInfo();
      break;
    case UpdaterScreen_NO_UPDATE:
      showNoUpdateInfo();
      break;
  }
  delay(250);
  return true;
}
