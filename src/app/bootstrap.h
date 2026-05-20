#ifndef BOOTSTRAP_H
#define BOOTSTRAP_H

#include "application.h"
#include "wifimanager.h"
#include "otamanager.h"
#include "systemconfig.h"
#include "levelsensor.h"

class Bootstrap
{
public:
  Bootstrap();
  Application &application();

private:
  WifiManager _wifiManager;
  OtaManager _otaManager;
  SystemConfig _systemConfig;
  LevelSensor _levelSensor;
  Application _app;
};

#endif // BOOTSTRAP_H
