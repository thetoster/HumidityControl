#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <stdlib.h>
#include "SSD1306.h" // alias for `#include "SSD1306Wire.h"`
#include "EnvLogic.h"
#include "MyServer.h"

SSD1306  display(0x3c, 5, 4);

void setup() {
  display.init();
  display.displayOn();
  display.normalDisplay();
  display.setContrast(128);
  display.setColor(WHITE);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.flipScreenVertically();
  Serial.begin(115200);
}

void normalMode() {
  envLogic.update();
  display.clear();
  display.setColor(WHITE);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_16);
  if (envLogic.isFanEnabled()) {
    display.drawString(0, 0, envLogic.getDisplayFan());
  }
  display.setFont(ArialMT_Plain_24);
  display.drawString(0, 16, envLogic.getDisplayHum());
  display.drawString(0, 37, envLogic.getDisplayTemp());
  display.display();
  delay(2000);
}

void configMode() {
  display.clear();
  display.setColor(WHITE);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_16);
  if (envLogic.isFanEnabled()) {
    display.drawString(0, 0, "Konfiguracja");
  }
  display.setFont(ArialMT_Plain_24);
  display.drawString(0, 16, myServer.getServerIp());
  display.drawString(0, 37, myServer.getPassword());
  display.display();
  delay(2000);
}

void loop() {
  if (myServer.isServerConfigured()) {
    normalMode();

  } else {
    configMode();
  }
}
