#ifndef TANKMODEL_H
#define TANKMODEL_H

#include "tanklevel.h"

// Pure conversion from sensor voltage to tank values. No Arduino dependency,
// so it is covered by the native unit tests.
namespace TankModel
{
float voltageToCurrentMa(float voltageV);

// Piecewise linear over SENSOR_CAL_TABLE: clamped below the first point,
// extrapolated with the last segment's slope above the last point.
float voltageToHeightCm(float voltageV);

float heightToPercent(float heightCm);
float heightToVolumeLiters(float heightCm);
float overflowLiters(float volumeLiters);

TankLevel fromVoltage(float voltageV);
} // namespace TankModel

#endif // TANKMODEL_H
