/*
 MIT License

 Copyright (c) 2018 Bartłomiej Żarnowski

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 SOFTWARE.

 LinearHeuristic.cpp
 Created on: Dec 1, 2018
 Author: Bartłomiej Żarnowski (Toster)
 */
#include "LinearHeuristic.h"

constexpr long TIME_TO_ADD_MIN = 20 * 60 * 1000;
constexpr int MAX_COUNT_OF_MIN_VALS = 6;

LinearHeuristic::LinearHeuristic(Fan& fan, std::vector<Measurement>& measurements)
: Heuristic(fan), measurements(measurements) {

}

void LinearHeuristic::update(int humidity) {
  //check for min know value?
  minValue = minValue > humidity ? humidity : minValue;
  long delta = millis() - lastUpdate;
  if (delta > 0) {
    timeToAddMinValue -= delta;
    if (timeToAddMinValue < 0) {
      storeMinValue();
    }
  }
  lastUpdate = millis();

  //detect raising slope
  float a = calcCoefficient();
  fan.shouldRun = a > 0.45;

  //fan is probably working, or flat readings, see if humidity is above known minimal value
  //if yes air is saturated and need some time to blow out all humidity but no
  //peak is detected, so we need to check known minimal values
  if ((a > 0) && (a < 0.1) && (fan.shouldRun == false)) {
    fan.shouldRun = isAboveMin(humidity);
  }
}

bool LinearHeuristic::isAboveMin(int humidity) {
  if (minValues.size() > 0) {
    int first = minValues[0];
    return (humidity - first) > 2;

  } else {
    return false;
  }
}

void LinearHeuristic::storeMinValue() {
  timeToAddMinValue = TIME_TO_ADD_MIN;
  minValues.push_back(minValue);
  minValue = 100;  //some big value :)
  //remove if too many
  if (minValues.size() > MAX_COUNT_OF_MIN_VALS) {
    minValues.erase(minValues.begin());
  }
}

void LinearHeuristic::calcMeans(float& x, float& y) {
  x = 0;
  y = 0;
  int32_t startT = measurements[0].timestamp;

  for(const Measurement m : measurements) {
    x += m.humidity;
    y += m.timestamp - startT;
  }
  x /= measurements.size();
  y /= measurements.size();
}

float LinearHeuristic::calcCoefficient() {
  if (measurements.size() == 0) {
      return 0;
  }
  float meanX, meanY;
  calcMeans(meanX, meanY);
  int32_t startT = measurements[0].timestamp;

  float bottom = 0;
  float top = 0;
  for (const Measurement m : measurements) {
    top += (m.humidity - meanX) * (m.timestamp - startT - meanY);
    bottom += (m.humidity - meanX) * (m.humidity - meanX);
  }
  return top / bottom;
}
