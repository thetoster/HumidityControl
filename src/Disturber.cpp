//
// Created by bartlomiejzarnowski on 18.07.18.
//

#include "Disturber.h"
#include "misc/Prefs.h"

Disturber::Disturber(Fan& fan) : fan(fan) {}

void Disturber::update(int humidity) {
  if (fan.shouldRun == false && humidity > prefs.storage.humidityTrigger) {
    offTime++;
    if (offTime == prefs.storage.disturberTriggerTime) {
      fan.shouldRun = true;
    }

  } else {
    offTime = 0;
  }
}
