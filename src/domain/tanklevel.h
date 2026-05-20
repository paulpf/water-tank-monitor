#ifndef TANKLEVEL_H
#define TANKLEVEL_H

struct TankLevel
{
  float currentMa;
  float levelPercent;

  // Outside 4-20mA range indicates wiring fault or sensor error
  bool isValid() const
  {
    return currentMa >= 3.8f && currentMa <= 20.5f;
  }
};

#endif // TANKLEVEL_H
