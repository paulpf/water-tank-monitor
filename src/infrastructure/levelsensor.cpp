#include "levelsensor.h"
#include "config.h"

static float interpolateLevel(float voltage)
{
  const int n = SENSOR_CAL_TABLE_SIZE;

  if (voltage <= SENSOR_CAL_TABLE[0].voltageV)
    return SENSOR_CAL_TABLE[0].actualPercent;

  if (voltage >= SENSOR_CAL_TABLE[n - 1].voltageV)
    return SENSOR_CAL_TABLE[n - 1].actualPercent;

  for (int i = 1; i < n; i++)
  {
    if (voltage <= SENSOR_CAL_TABLE[i].voltageV)
    {
      float v0 = SENSOR_CAL_TABLE[i - 1].voltageV;
      float p0 = SENSOR_CAL_TABLE[i - 1].actualPercent;
      float v1 = SENSOR_CAL_TABLE[i].voltageV;
      float p1 = SENSOR_CAL_TABLE[i].actualPercent;
      float t  = (voltage - v0) / (v1 - v0);
      return p0 + t * (p1 - p0);
    }
  }

  return SENSOR_CAL_TABLE[n - 1].actualPercent;
}

#if defined(ARDUINO_ARCH_ESP8266) || defined(ARDUINO_ARCH_ESP32)

bool LevelSensor::setup()
{
  _ads.setGain(GAIN_ONE);
  return _ads.begin(SENSOR_ADS_I2C_ADDR);
}

TankLevel LevelSensor::read()
{
  int16_t raw    = _ads.readADC_SingleEnded(SENSOR_ADS_CHANNEL);
  float voltage  = _ads.computeVolts(raw);
  float currentMa    = (voltage / SENSOR_VREF) * 16.0f + 4.0f;
  float levelPercent = interpolateLevel(voltage);

  return TankLevel{currentMa, levelPercent};
}

#else

// Native test stubs
bool LevelSensor::setup() { return true; }

TankLevel LevelSensor::read() { return TankLevel{4.0f, 0.0f}; }

#endif
