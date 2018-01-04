/*
  SHT21 - Library for ESP8266 and Arduino for the Sensirion Temperature and Humidity sensor

  Created by Markus Ulsass, Hamburg, Germany
  github@tradewire.de
  23-5-2016
  https://github.com/markbeee/SHT21

  With credits to:

  HTU21D Humidity Sensor Library
  By: Nathan Seidle
  SparkFun Electronics
  Date: September 22nd, 2013
  License: This code is public domain but you buy me a beer if you use this and we meet someday (Beerware license).

*/

#ifndef SHT21_H
#define SHT21_H

#include <Wire.h>
#include <Arduino.h>

#define SHT21_ADDRESS 0x40  //I2C address for the sensor

class SHT21 {

public:
  void begin();
  float getHumidity(void);
  float getTemperature(void);

private:
  uint16_t readSHT21(uint8_t command);
  uint8_t read8(uint8_t command);
  void write8(uint8_t reg, uint8_t value);
};

#endif
