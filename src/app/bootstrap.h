#ifndef BOOTSTRAP_H
#define BOOTSTRAP_H

#include "application.h"
#include "wifimanager.h"
#include "otamanager.h"
#include "systemconfig.h"
#include "levelsensor.h"
#include "mqttmanager.h"
#include "connectivitycoordinator.h"

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
  MqttManager _mqttManager;
  ConnectivityCoordinator _connectivityCoordinator;
  Application _app;
};

#endif // BOOTSTRAP_H
