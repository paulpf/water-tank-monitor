#ifndef APPLICATION_H
#define APPLICATION_H

#include "wifimanager.h"
#include "otamanager.h"
#include "systemconfig.h"
#include "ilevelsensor.h"
#include "mqttmanager.h"
#include "connectivitycoordinator.h"
#include "watchdog.h"

class Application
{
public:
  Application(WifiManager &wifiManager, OtaManager &otaManager,
              SystemConfig &systemConfig, ILevelSensor &levelSensor,
              MqttManager &mqttManager,
              ConnectivityCoordinator &connectivityCoordinator,
              Watchdog &watchdog);

  void setup();
  void loop();

private:
  enum class StartupState
  {
    WAITING_FOR_WIFI,
    RUNNING
  };

  void handleStartup();
  void readSensor();
  void publishLevel();
  void publishHealth();
  void handleMqttMessage(char *topic, uint8_t *payload, unsigned int length);

  WifiManager &_wifiManager;
  OtaManager &_otaManager;
  SystemConfig &_systemConfig;
  ILevelSensor &_levelSensor;
  MqttManager &_mqttManager;
  ConnectivityCoordinator &_connectivityCoordinator;
  Watchdog &_watchdog;
  unsigned long _lastStatusPrint;
  unsigned long _lastSensorRead;
  unsigned long _lastPublish;
  unsigned long _lastRssiPublish;
  unsigned long _startupWaitStart;
  StartupState _startupState;
  bool _mqttWasConnected;
  bool _otaSetupDone;

  // Most recently read sensor values, decoupled from publish cadence.
  TankLevel _lastLevel;
  bool _lastSensorValid;
};

#endif // APPLICATION_H
