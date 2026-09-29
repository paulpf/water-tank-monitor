#ifndef APPLICATION_H
#define APPLICATION_H

#include "intervaltimer.h"
#include "levelsensor.h"
#include "mqttmanager.h"
#include "otamanager.h"
#include "systemconfig.h"
#include "watchdog.h"
#include "wifimanager.h"

class Application
{
public:
  Application(WifiManager &wifiManager, OtaManager &otaManager,
              SystemConfig &systemConfig, LevelSensor &levelSensor,
              MqttManager &mqttManager, Watchdog &watchdog);

  void setup();
  void loop();

private:
  void onMqttConnected(uint32_t now);
  void publishRssiIfDue(uint32_t now);
  void readSensorIfDue(uint32_t now);
  void publishLevelIfDue(uint32_t now);
  void printStatusIfDue(uint32_t now);

  void readSensor();
  void publishLevel();
  void publishHealth();
  void publishInterval(const char *topic, uint32_t intervalMs);

  void handleMqttMessage(char *topic, uint8_t *payload, unsigned int length);
  void applyInterval(const uint8_t *payload, unsigned int length, uint32_t &target,
                     const char *stateTopic, const char *logMessage);

  WifiManager &_wifiManager;
  OtaManager &_otaManager;
  SystemConfig &_systemConfig;
  LevelSensor &_levelSensor;
  MqttManager &_mqttManager;
  Watchdog &_watchdog;

  IntervalTimer _rssiTimer;
  IntervalTimer _readTimer;
  IntervalTimer _publishTimer;
  IntervalTimer _statusTimer;

  // Most recently read sensor values, decoupled from publish cadence.
  TankLevel _lastLevel;
  bool _lastSensorValid;
};

#endif // APPLICATION_H
