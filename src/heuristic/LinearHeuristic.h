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

 LinearHeuristic.h
 Created on: Dec 1, 2018
 Author: Bartłomiej Żarnowski (Toster)
 */
#ifndef LinearHeuristic_hpp
#define LinearHeuristic_hpp

#include "Heuristic.h"
#include "Measurement.h"
#include <vector>

class LinearHeuristic : public Heuristic {
  public:
    LinearHeuristic(Fan& fan, std::vector<Measurement>& measurements);
    virtual ~LinearHeuristic() override = default;

    void update(int humidity) override;
  private:
    std::vector<Measurement>& measurements;
    std::vector<int8_t> minValues;
    unsigned long lastUpdate = 0;
    long timeToAddMinValue = 0;
    int8_t minValue = 0;

    void storeMinValue();
    float calcCoefficient();
    bool isAboveMin(int humidity);
    void calcMeans(float& x, float& y);
};

#endif /* LinearHeuristic_hpp */
