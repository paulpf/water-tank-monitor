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

static float percentToHeight(float levelPercent)
{
  return TANK_MIN_HEIGHT_CM + (levelPercent / 100.0f) * (TANK_DRAIN_HEIGHT_CM - TANK_MIN_HEIGHT_CM);
}

static float heightToVolume(float heightCm)
{
  constexpr float M_PI_VALUE = 3.14159265f;

  if (heightCm <= TANK_MIN_HEIGHT_CM)
    return 408.0f;  // Min volume at sensor height (13 cm)

  if (heightCm >= TANK_DRAIN_HEIGHT_CM)
    return 6500.0f;  // Max volume at drain outlet (206 cm)

  // Cylinder: 0–198 cm
  if (heightCm <= TANK_CYLINDER_HEIGHT_CM)
  {
    float heightM = heightCm / 100.0f;
    float volumeM3 = M_PI_VALUE * TANK_RADIUS_M * TANK_RADIUS_M * heightM;
    return volumeM3 * 1000.0f;  // Convert m³ to liters
  }

  // Cone: 198–206 cm (linear interpolation between cylinder and full)
  float cylinderVolume = M_PI_VALUE * TANK_RADIUS_M * TANK_RADIUS_M * (TANK_CYLINDER_HEIGHT_CM / 100.0f) * 1000.0f;
  float coneProgress = (heightCm - TANK_CYLINDER_HEIGHT_CM) / (TANK_DRAIN_HEIGHT_CM - TANK_CYLINDER_HEIGHT_CM);
  float coneVolume = 6500.0f - cylinderVolume;
  return cylinderVolume + (coneProgress * coneVolume);
}

#if defined(ARDUINO_ARCH_ESP8266) || defined(ARDUINO_ARCH_ESP32)

bool LevelSensor::setup()
{
  _ads.setGain(GAIN_ONE);
  _ready = _ads.begin(SENSOR_ADS_I2C_ADDR);
  return _ready;
}

TankLevel LevelSensor::read()
{
  if (!_ready)
    return TankLevel{0.0f, 0.0f, 0.0f, 13.0f, 408.0f};

  int16_t raw    = _ads.readADC_SingleEnded(SENSOR_ADS_CHANNEL);
  float voltage  = _ads.computeVolts(raw);
  float currentMa    = (voltage / SENSOR_VREF) * 16.0f + 4.0f;
  float levelPercent = interpolateLevel(voltage);
  float heightCm     = percentToHeight(levelPercent);
  float volumeLiters = heightToVolume(heightCm);

  return TankLevel{currentMa, voltage, levelPercent, heightCm, volumeLiters};
}

bool LevelSensor::isReady() const { return _ready; }

#else

// Native test stubs
bool LevelSensor::setup() { return true; }

TankLevel LevelSensor::read() { return TankLevel{4.0f, 0.0f, 0.0f, 13.0f, 408.0f}; }

bool LevelSensor::isReady() const { return true; }

#endif
