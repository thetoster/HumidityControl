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

 AdaptiveHeuristic2.cpp
 Created on: Jul 19, 2018
 Author: Bartłomiej Żarnowski (Toster)
 */

#include "AdaptiveHeuristic2.h"
#include "misc/Prefs.h"
#include <numeric>
#include <algorithm>

AdaptiveHeuristic2::AdaptiveHeuristic2(Fan &fan) : Heuristic(fan), disturber(Disturber(fan)) {}

void AdaptiveHeuristic2::update(int humidity) {
  samples.push_back(humidity);

  if (samples.size() == prefs.storage.noSamples) {
    double mean, stdDev;
    calcMeanAndStdDev(mean, stdDev);

    fan.shouldRun = significantDiff(mean, baseMean);// or significantDiff(stdDev, baseStdDev);
    baseMean = mean;
    baseStdDev = stdDev;

    samples.clear();
  }

  //random environment trigger changer
  if (prefs.storage.useDisturber != 0) {
  	disturber.update(humidity);
  }
}

void AdaptiveHeuristic2::calcMeanAndStdDev(double &mean, double &stdev) {
  mean = std::accumulate(samples.begin(), samples.end(), 0.0) / samples.size();

  double accum = 0.0;
  for(auto i = samples.begin(); i != samples.end(); i++) {
    accum += (*i - mean) * (*i - mean);
  };

  stdev = sqrt(accum / (samples.size()-1));
}

bool AdaptiveHeuristic2::significantDiff(double val1, double val2) {
  val1 = val1 == 0 ? 1 : val1;
  val1 = (val2 * 100 / val1);
  val1 = abs(val1 - 100);
  return val1 > 3;
}
