#ifndef LEVELSENSOR_H
#define LEVELSENSOR_H

#include "ilevelsensor.h"

#if defined(ARDUINO_ARCH_ESP8266) || defined(ARDUINO_ARCH_ESP32)
#include <Adafruit_ADS1X15.h>
#endif

class LevelSensor : public ILevelSensor
{
public:
  bool setup() override;
  TankLevel read() override;

private:
#if defined(ARDUINO_ARCH_ESP8266) || defined(ARDUINO_ARCH_ESP32)
  Adafruit_ADS1115 _ads;
#endif
};

#endif // LEVELSENSOR_H
