#include "levelsensor.h"
#include "config.h"
#include "tankmodel.h"

bool LevelSensor::setup()
{
  _ads.setGain(GAIN_ONE);
  _ready = _ads.begin(SENSOR_ADS_I2C_ADDR);
  return _ready;
}

TankLevel LevelSensor::read()
{
  if (!_ready)
    return TankLevel::notReady();

  int16_t raw = _ads.readADC_SingleEnded(SENSOR_ADS_CHANNEL);
  return TankModel::fromVoltage(_ads.computeVolts(raw));
}

bool LevelSensor::isReady() const { return _ready; }
