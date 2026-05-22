#ifndef APPLICATION_H
#define APPLICATION_H

#include "wifimanager.h"
#include "otamanager.h"
#include "systemconfig.h"
#include "ilevelsensor.h"
#include "mqttmanager.h"
#include "connectivitycoordinator.h"

class Application
{
public:
  Application(WifiManager &wifiManager, OtaManager &otaManager,
              SystemConfig &systemConfig, ILevelSensor &levelSensor,
              MqttManager &mqttManager,
              ConnectivityCoordinator &connectivityCoordinator);

  void setup();
  void loop();

private:
  enum class StartupState
  {
    WAITING_FOR_WIFI,
    RUNNING
  };

  void handleStartup();
  void readAndPublishLevel();

  WifiManager &_wifiManager;
  OtaManager &_otaManager;
  SystemConfig &_systemConfig;
  ILevelSensor &_levelSensor;
  MqttManager &_mqttManager;
  ConnectivityCoordinator &_connectivityCoordinator;
  unsigned long _lastStatusPrint;
  unsigned long _lastSensorRead;
  unsigned long _lastRssiPublish;
  unsigned long _startupWaitStart;
  StartupState _startupState;
  bool _mqttWasConnected;
};

#endif // APPLICATION_H
