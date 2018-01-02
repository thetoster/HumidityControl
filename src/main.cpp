#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <stdlib.h>
#include <SSD1306.h> // alias for `#include "SSD1306Wire.h"`
#include "EnvLogic.h"
#include "MyServer.h"
#include "Updater.h"

SSD1306  display(0x3c, 5, 4);

void setup() {
  display.init();
  display.displayOn();
  display.normalDisplay();
  display.setContrast(128);
  display.setColor(WHITE);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.flipScreenVertically();
}

void normalMode() {
  envLogic.update();
  myServer.update();
  display.clear();
  display.setColor(WHITE);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_16);
  if (envLogic.isFanEnabled()) {
    display.drawString(0, 0, envLogic.getDisplayFan());
  } else {
    //show network state
    display.drawString(0, 0, myServer.getStatus());
  }
  display.setFont(ArialMT_Plain_24);
  display.drawString(0, 16, envLogic.getDisplayHum());
  display.drawString(0, 37, envLogic.getDisplayTemp());
  display.display();
  delay(200);
}

void configMode() {
  display.clear();
  display.setColor(WHITE);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_16);
  display.drawString(0, 0, "Konfiguracja");
  display.drawString(0, 16, myServer.getServerIp());
  display.drawString(0, 37, myServer.getPassword());
  display.display();
  myServer.update();
  delay(200);
}

void loop() {
  //updater has it's own display management
  if (updater.update()) {
    return;
  }

  //no update in progress
  if (myServer.isServerConfigured()) {
    normalMode();

  } else {
    configMode();
  }
}
