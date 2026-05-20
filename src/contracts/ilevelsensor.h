#ifndef ILEVELSENSOR_H
#define ILEVELSENSOR_H

#include "tanklevel.h"

class ILevelSensor
{
public:
  virtual ~ILevelSensor() = default;
  virtual bool setup() = 0;
  virtual TankLevel read() = 0;
};

#endif // ILEVELSENSOR_H
