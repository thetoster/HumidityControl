#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <stdlib.h>
#include <SSD1306.h> // alias for `#include "SSD1306Wire.h"`
#include "EnvLogic.h"
#include "MyServer.h"
#include "Updater.h"
#include "misc/Prefs.h"
#include "periphery/Buttons.h"
#include "misc/lfont.h"

#define TIME_TO_RESET (1000 * 24 * 3600)

SSD1306  display(0x3c, 5, 4);

void setup() {
  Serial.begin(115200);
  display.init();
  display.displayOn();
  display.normalDisplay();
  display.setContrast(128);
  display.setColor(WHITE);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_16);
  display.drawString(0, 0, "Bootowanie");
  display.drawString(0, 16, versionString);
  display.display();

  prefs.load();
  myServer.restart();

  //dump prefs
  if (prefs.hasPrefs()) {
  	Serial.print("SSID:");
  	Serial.println(prefs.storage.ssid);
    Serial.print("In Net Name:");
    Serial.println(prefs.storage.inNetworkName);
    Serial.print("Password:");
    Serial.println(prefs.storage.password);
  } else {
    Serial.println("No prefs!");
  }
  Serial.flush();
}

void normalMode() {
  envLogic.update();
  myServer.update();
  display.clear();
  display.setColor(WHITE);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_16);
  bool hCenter = false;
  if (envLogic.isFanRunning()) {
    display.drawString(0, 0, envLogic.getDisplayFan());
    if (envLogic.getDisplayFan().length() == 0) {
      hCenter = true;
    }
  } else {
    //show network state
    display.drawString(0, 0, myServer.getStatus());
    if (myServer.getStatus().length() == 0) {
      hCenter = true;
    }
  }
  display.setFont(Chewy_Regular_42);
  String str = envLogic.getDisplayHum();
  int w = display.getStringWidth(str);
  display.drawString((display.width() - w) / 2,
      hCenter ? (display.getHeight() - 42) / 2 : 16,
      str);
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

  buttons.update();

  //no update in progress
  if (myServer.isServerConfigured()) {
    normalMode();

  } else {
    configMode();
  }
}
