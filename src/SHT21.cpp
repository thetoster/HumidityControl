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

#include <Wire.h>
#include "SHT21.h"

#define TRIGGER_TEMP_MEASURE_NOHOLD  0xF3
#define TRIGGER_HUMD_MEASURE_NOHOLD  0xF5
#define USER_REGISTER_WRITE   0xE6    //Write user register
#define USER_REGISTER_READ    0xE7    //Read  user register
#define HEATER_OFF 0xFB

void SHT21::begin(void){
  Wire.begin();

  //Turn off the heater
  uint8_t userRegisterData = read8(USER_REGISTER_READ);
  userRegisterData &= HEATER_OFF;
  write8(USER_REGISTER_WRITE, userRegisterData);
}

float SHT21::getHumidity(void)
{
  const double d = readSHT21(TRIGGER_HUMD_MEASURE_NOHOLD);
  return (-6.0 + 125.0 * d / 65536.0);
}

float SHT21::getTemperature(void)
{
  const double d = readSHT21(TRIGGER_TEMP_MEASURE_NOHOLD);
  return (-46.85 + 175.72 * d / 65536.0);
}

void SHT21::write8(uint8_t reg, uint8_t value)
{
  Wire.beginTransmission(SHT21_ADDRESS);
  Wire.write(reg);
  Wire.write(value);
  Wire.endTransmission();
}

/**************************************************************************/
/*
    Reads 8 bit value from the sensor, over I2C
*/
/**************************************************************************/
uint8_t SHT21::read8(uint8_t command)
{
  Wire.beginTransmission(SHT21_ADDRESS);
  Wire.write(command);
  Wire.endTransmission();
  delay(100);

  Wire.requestFrom(SHT21_ADDRESS, 1);
  while(Wire.available() < 1) {
    delay(1);
  }
  return Wire.read();
}

uint16_t SHT21::readSHT21(uint8_t command)
{
  uint16_t result;

  Wire.beginTransmission(SHT21_ADDRESS);
  Wire.write(command);
  Wire.endTransmission();
  delay(100);

  Wire.requestFrom(SHT21_ADDRESS, 3);
  while(Wire.available() < 3) {
    delay(1);
  }

  // return result
  result = ((Wire.read()) << 8);
  result += Wire.read();
  result &= ~0x0003;   // clear two low bits (status bits)
  return result;
}
