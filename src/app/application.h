#ifndef APPLICATION_H
#define APPLICATION_H

#include "wifimanager.h"
#include "otamanager.h"
#include "systemconfig.h"
#include "ilevelsensor.h"

class Application
{
public:
  Application(WifiManager &wifiManager, OtaManager &otaManager,
              SystemConfig &systemConfig, ILevelSensor &levelSensor);

  void setup();
  void loop();

private:
  enum class StartupState
  {
    WAITING_FOR_WIFI,
    RUNNING
  };

  void handleStartup();
  void readAndLogLevel();

  WifiManager &_wifiManager;
  OtaManager &_otaManager;
  SystemConfig &_systemConfig;
  ILevelSensor &_levelSensor;
  unsigned long _lastStatusPrint;
  unsigned long _lastSensorRead;
  unsigned long _startupWaitStart;
  StartupState _startupState;
};

#endif // APPLICATION_H
