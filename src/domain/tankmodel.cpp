#include "tankmodel.h"
#include "config.h"

namespace TankModel
{

float voltageToCurrentMa(float voltageV)
{
  return (voltageV / SENSOR_VREF) * 16.0f + 4.0f;
}

float voltageToHeightCm(float voltageV)
{
  if (voltageV <= SENSOR_CAL_TABLE[0].voltageV)
  {
    return SENSOR_CAL_TABLE[0].heightCm;
  }

  int i = 1;
  while (i < SENSOR_CAL_TABLE_SIZE - 1 && voltageV > SENSOR_CAL_TABLE[i].voltageV)
  {
    i++;
  }

  const SensorCalPoint &p0 = SENSOR_CAL_TABLE[i - 1];
  const SensorCalPoint &p1 = SENSOR_CAL_TABLE[i];
  const float t = (voltageV - p0.voltageV) / (p1.voltageV - p0.voltageV);
  return p0.heightCm + t * (p1.heightCm - p0.heightCm);
}

float heightToPercent(float heightCm)
{
  return ((heightCm - TANK_MIN_HEIGHT_CM) / (TANK_DRAIN_HEIGHT_CM - TANK_MIN_HEIGHT_CM)) * 100.0f;
}

float heightToVolumeLiters(float heightCm)
{
  constexpr float PI_F = 3.14159265f;

  if (heightCm <= TANK_MIN_HEIGHT_CM)
  {
    return TANK_MIN_VOLUME_L;
  }

  // Cylinder: 0–198 cm
  if (heightCm <= TANK_CYLINDER_HEIGHT_CM)
  {
    const float heightM = heightCm / 100.0f;
    const float volumeM3 = PI_F * TANK_RADIUS_M * TANK_RADIUS_M * heightM;
    return volumeM3 * 1000.0f;
  }

  // Cone: 198–210 cm, extrapolated at the same rate beyond the drain outlet
  // so water above nominal capacity (210 cm) still shows up as overflow.
  const float cylinderVolume = PI_F * TANK_RADIUS_M * TANK_RADIUS_M * (TANK_CYLINDER_HEIGHT_CM / 100.0f) * 1000.0f;
  const float coneProgress = (heightCm - TANK_CYLINDER_HEIGHT_CM) / (TANK_DRAIN_HEIGHT_CM - TANK_CYLINDER_HEIGHT_CM);
  const float coneVolume = TANK_NOMINAL_VOLUME_L - cylinderVolume;
  return cylinderVolume + (coneProgress * coneVolume);
}

float overflowLiters(float volumeLiters)
{
  return volumeLiters > TANK_NOMINAL_VOLUME_L ? volumeLiters - TANK_NOMINAL_VOLUME_L : 0.0f;
}

TankLevel fromVoltage(float voltageV)
{
  const float heightCm = voltageToHeightCm(voltageV);
  const float volumeLiters = heightToVolumeLiters(heightCm);
  return TankLevel{voltageToCurrentMa(voltageV), voltageV, heightToPercent(heightCm),
                   heightCm, volumeLiters, overflowLiters(volumeLiters)};
}

} // namespace TankModel
