#include "levelsensor.h"
#include "config.h"

static float interpolateHeightCm(float voltage)
{
  const int n = SENSOR_CAL_TABLE_SIZE;

  if (voltage <= SENSOR_CAL_TABLE[0].voltageV)
    return SENSOR_CAL_TABLE[0].heightCm;

  for (int i = 1; i < n; i++)
  {
    if (voltage <= SENSOR_CAL_TABLE[i].voltageV)
    {
      float v0 = SENSOR_CAL_TABLE[i - 1].voltageV;
      float h0 = SENSOR_CAL_TABLE[i - 1].heightCm;
      float v1 = SENSOR_CAL_TABLE[i].voltageV;
      float h1 = SENSOR_CAL_TABLE[i].heightCm;
      float t  = (voltage - v0) / (v1 - v0);
      return h0 + t * (h1 - h0);
    }
  }

  // Beyond the last calibration point: extrapolate using the last segment's
  // slope instead of clamping, so overflow readings aren't lost.
  float v0 = SENSOR_CAL_TABLE[n - 2].voltageV;
  float h0 = SENSOR_CAL_TABLE[n - 2].heightCm;
  float v1 = SENSOR_CAL_TABLE[n - 1].voltageV;
  float h1 = SENSOR_CAL_TABLE[n - 1].heightCm;
  float t  = (voltage - v0) / (v1 - v0);
  return h0 + t * (h1 - h0);
}

static float heightToPercent(float heightCm)
{
  return ((heightCm - TANK_MIN_HEIGHT_CM) / (TANK_DRAIN_HEIGHT_CM - TANK_MIN_HEIGHT_CM)) * 100.0f;
}

static float heightToVolume(float heightCm)
{
  constexpr float M_PI_VALUE = 3.14159265f;

  if (heightCm <= TANK_MIN_HEIGHT_CM)
    return 408.0f;  // Min volume at sensor height (13 cm)

  // Cylinder: 0–198 cm
  if (heightCm <= TANK_CYLINDER_HEIGHT_CM)
  {
    float heightM = heightCm / 100.0f;
    float volumeM3 = M_PI_VALUE * TANK_RADIUS_M * TANK_RADIUS_M * heightM;
    return volumeM3 * 1000.0f;  // Convert m³ to liters
  }

  // Cone: 198–210 cm, extrapolated at the same rate beyond the drain outlet
  // so water above nominal capacity (210 cm) still shows up as overflow.
  float cylinderVolume = M_PI_VALUE * TANK_RADIUS_M * TANK_RADIUS_M * (TANK_CYLINDER_HEIGHT_CM / 100.0f) * 1000.0f;
  float coneProgress = (heightCm - TANK_CYLINDER_HEIGHT_CM) / (TANK_DRAIN_HEIGHT_CM - TANK_CYLINDER_HEIGHT_CM);
  float coneVolume = 6500.0f - cylinderVolume;
  return cylinderVolume + (coneProgress * coneVolume);
}

static float overflowVolume(float volumeLiters)
{
  return volumeLiters > 6500.0f ? volumeLiters - 6500.0f : 0.0f;
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
    return TankLevel{0.0f, 0.0f, 0.0f, 13.0f, 408.0f, 0.0f};

  int16_t raw    = _ads.readADC_SingleEnded(SENSOR_ADS_CHANNEL);
  float voltage  = _ads.computeVolts(raw);
  float currentMa          = (voltage / SENSOR_VREF) * 16.0f + 4.0f;
  float heightCm           = interpolateHeightCm(voltage);
  float levelPercent       = heightToPercent(heightCm);
  float volumeLiters       = heightToVolume(heightCm);
  float overflowLiters     = overflowVolume(volumeLiters);

  return TankLevel{currentMa, voltage, levelPercent, heightCm, volumeLiters, overflowLiters};
}

bool LevelSensor::isReady() const { return _ready; }

#else

// Native test stubs
bool LevelSensor::setup() { return true; }

TankLevel LevelSensor::read() { return TankLevel{4.0f, 0.0f, 0.0f, 13.0f, 408.0f, 0.0f}; }

bool LevelSensor::isReady() const { return true; }

#endif
