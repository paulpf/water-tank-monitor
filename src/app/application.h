#ifndef APPLICATION_H
#define APPLICATION_H

#include "wifimanager.h"
#include "otamanager.h"
#include "systemconfig.h"
#include "levelsensor.h"
#include "mqttmanager.h"
#include "watchdog.h"

class Application
{
public:
  Application(WifiManager &wifiManager, OtaManager &otaManager,
              SystemConfig &systemConfig, LevelSensor &levelSensor,
              MqttManager &mqttManager, Watchdog &watchdog);

  void setup();
  void loop();

private:
  void onMqttConnected(unsigned long now);
  void readSensor();
  void publishLevel();
  void publishHealth();
  void handleMqttMessage(char *topic, uint8_t *payload, unsigned int length);

  WifiManager &_wifiManager;
  OtaManager &_otaManager;
  SystemConfig &_systemConfig;
  LevelSensor &_levelSensor;
  MqttManager &_mqttManager;
  Watchdog &_watchdog;
  unsigned long _lastStatusPrint;
  unsigned long _lastSensorRead;
  unsigned long _lastPublish;
  unsigned long _lastRssiPublish;

  // Most recently read sensor values, decoupled from publish cadence.
  TankLevel _lastLevel;
  bool _lastSensorValid;
};

#endif // APPLICATION_H
