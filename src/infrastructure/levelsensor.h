#ifndef LEVELSENSOR_H
#define LEVELSENSOR_H

#include <Adafruit_ADS1X15.h>
#include "tanklevel.h"

class LevelSensor
{
public:
  bool setup();
  TankLevel read();
  bool isReady() const;

private:
  Adafruit_ADS1115 _ads;
  bool _ready = false;
};

#endif // LEVELSENSOR_H
