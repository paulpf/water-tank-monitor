#ifndef LEVELSENSOR_H
#define LEVELSENSOR_H

#include <Adafruit_ADS1X15.h>
#include "ilevelsensor.h"

class LevelSensor : public ILevelSensor
{
public:
  bool setup() override;
  TankLevel read() override;
  bool isReady() const override;

private:
  Adafruit_ADS1115 _ads;
  bool _ready = false;
};

#endif // LEVELSENSOR_H
