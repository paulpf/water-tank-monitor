#ifndef TANKLEVEL_H
#define TANKLEVEL_H

#include "config.h"

struct TankLevel
{
  // Placeholder before the first reading or while the ADC is missing;
  // isValid() is false because currentMa is 0.
  static TankLevel notReady()
  {
    return TankLevel{0.0f, 0.0f, 0.0f, TANK_MIN_HEIGHT_CM, TANK_MIN_VOLUME_L, 0.0f};
  }

  float currentMa;
  float voltageV;
  float levelPercent;
  float heightCm;
  float volumeLiters;
  float overflowLiters;

  // Outside 4-20mA range indicates wiring fault or sensor error
  bool isValid() const
  {
    return currentMa >= 3.8f && currentMa <= 20.5f;
  }
};

#endif // TANKLEVEL_H
