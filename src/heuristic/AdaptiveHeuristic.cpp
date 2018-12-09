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

 AdaptiveHeuristic.cpp
 Created on: Jul 18, 2018
 Author: Bartłomiej Żarnowski (Toster)
 */

#include "AdaptiveHeuristic.h"
#include "misc/Prefs.h"
#include <numeric>
#include <algorithm>

AdaptiveHeuristic::AdaptiveHeuristic(Fan &fan) : Heuristic(fan), disturber(Disturber(fan)) {}

void AdaptiveHeuristic::update(int humidity) {
  samples.push_back(humidity);

  if (samples.size() == prefs.storage.noSamples) {
    double mean, stdDev;
    calcMeanAndStdDev(mean, stdDev);

    fan.shouldRun = significantMeanChange(mean, stdDev);
    baseMean = mean;
    baseStdDev = stdDev;

    samples.clear();
  }

  //random environment trigger changer
  if (prefs.storage.useDisturber != 0) {
  	disturber.update(humidity);
  }
}

void AdaptiveHeuristic::calcMeanAndStdDev(double &mean, double &stdev) {
  mean = std::accumulate(samples.begin(), samples.end(), 0.0) / samples.size();

  double accum = 0.0;
  for(auto i = samples.begin(); i != samples.end(); i++) {
    accum += (*i - mean) * (*i - mean);
  };

  stdev = sqrt(accum / (samples.size()-1));
}

bool AdaptiveHeuristic::significantMeanChange(double mean, double stdDev) {
  double ss = baseStdDev < 0.1 ? baseMean / 12 : baseStdDev;
  return (abs(mean - baseMean) > 2 * ss);
}
