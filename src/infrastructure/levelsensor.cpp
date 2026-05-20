#include "levelsensor.h"
#include "config.h"

#if defined(ARDUINO_ARCH_ESP8266) || defined(ARDUINO_ARCH_ESP32)

bool LevelSensor::setup()
{
  _ads.setGain(GAIN_ONE);
  return _ads.begin(SENSOR_ADS_I2C_ADDR);
}

TankLevel LevelSensor::read()
{
  int16_t raw = _ads.readADC_SingleEnded(SENSOR_ADS_CHANNEL);
  float voltage = _ads.computeVolts(raw);

  float levelPercent = (voltage / SENSOR_VREF) * 100.0f;
  if (levelPercent < 0.0f)   levelPercent = 0.0f;
  if (levelPercent > 100.0f) levelPercent = 100.0f;

  float currentMa = (levelPercent / 100.0f) * 16.0f + 4.0f;
  return TankLevel{currentMa, levelPercent};
}

#else

// Native test stubs
bool LevelSensor::setup() { return true; }

TankLevel LevelSensor::read() { return TankLevel{4.0f, 0.0f}; }

#endif
