#include "application.h"
#include "trace.h"
#include "tanklevel.h"
#include "config.h"
#include <ESP8266WiFi.h>

#include "../../../_config/MqttConfig.h"
#include "../../../_secrets/MqttSecret.h"
#include "../../../_secrets/OtaSecret.h"
#include "../../../_secrets/WifiSecret.h"

Application::Application(WifiManager &wifiManager, OtaManager &otaManager,
                         SystemConfig &systemConfig, ILevelSensor &levelSensor,
                         MqttManager &mqttManager,
                         ConnectivityCoordinator &connectivityCoordinator)
    : _wifiManager(wifiManager), _otaManager(otaManager),
      _systemConfig(systemConfig), _levelSensor(levelSensor),
      _mqttManager(mqttManager),
      _connectivityCoordinator(connectivityCoordinator),
      _lastStatusPrint(0), _lastSensorRead(0), _lastRssiPublish(0), _startupWaitStart(0),
      _startupState(StartupState::WAITING_FOR_WIFI),
      _mqttWasConnected(false)
{
}

void Application::setup()
{
  Trace::log(TraceLevel::INFO, "Application setup started");

  if (!_levelSensor.setup())
  {
    Trace::log(TraceLevel::ERROR, "ADS1115 not found - check I2C wiring (D1=SCL, D2=SDA)");
  }
  else
  {
    Trace::log(TraceLevel::INFO, "ADS1115 ready");
  }

  _wifiManager.setup(WIFI_SSID, WIFI_PWD, DEVICE_NAME);
  _mqttManager.setup(MQTT_SERVER_IP, MQTT_SERVER_PORT, MQTT_USER, MQTT_PWD, DEVICE_NAME);

  _startupWaitStart = millis();
  _startupState = StartupState::WAITING_FOR_WIFI;
  Trace::log(TraceLevel::INFO, "Startup is non-blocking, waiting for WiFi in main loop");

  Trace::log(TraceLevel::INFO, "Application setup complete");
  _lastStatusPrint = millis();
}

void Application::loop()
{
  unsigned long currentTime = millis();

  handleStartup();

  _wifiManager.loop();
  _otaManager.loop();
  _connectivityCoordinator.handleEvents();
  _mqttManager.loop();

  bool mqttConnectedNow = _mqttManager.isConnected();
  if (mqttConnectedNow && !_mqttWasConnected)
  {
    Trace::log(TraceLevel::INFO, "MQTT connected - publishing initial values");
    readAndPublishLevel();
    _lastSensorRead = currentTime;
  }
  _mqttWasConnected = mqttConnectedNow;

  if (_mqttManager.isConnected() &&
      currentTime - _lastRssiPublish >= MQTT_RSSI_INTERVAL_MS)
  {
    _lastRssiPublish = currentTime;
    char payload[8];
    snprintf(payload, sizeof(payload), "%d", WiFi.RSSI());
    _mqttManager.publish(MQTT_TOPIC_RSSI, payload);
  }

  if (currentTime - _lastSensorRead >= SENSOR_READ_INTERVAL_MS)
  {
    _lastSensorRead = currentTime;
    readAndPublishLevel();
  }

  if (currentTime - _lastStatusPrint >= STATUS_PRINT_INTERVAL_MS)
  {
    _lastStatusPrint = currentTime;
    if (_wifiManager.isConnected())
    {
      Trace::log(TraceLevel::INFO,
                 _mqttManager.isConnected() ? "WiFi OK, MQTT OK" : "WiFi OK, MQTT disconnected");
    }
    else
    {
      Trace::log(TraceLevel::INFO, "WiFi disconnected");
    }
  }
}

void Application::readAndPublishLevel()
{
  TankLevel level = _levelSensor.read();

  if (!level.isValid())
  {
    Trace::log(TraceLevel::WARNING, "Sensor out of range - check wiring");
    return;
  }

  char buf[64];
  snprintf(buf, sizeof(buf), "Tank: %.1f%% (%.2f mA)", level.levelPercent, level.currentMa);
  Trace::log(TraceLevel::INFO, buf);

  if (_mqttManager.isConnected())
  {
    char payload[16];
    snprintf(payload, sizeof(payload), "%.1f", level.levelPercent);
    _mqttManager.publishRetained(MQTT_TOPIC_LEVEL_PERCENT, payload);

    snprintf(payload, sizeof(payload), "%.2f", level.currentMa);
    _mqttManager.publishRetained(MQTT_TOPIC_CURRENT_MA, payload);
  }
}

void Application::handleStartup()
{
  if (_startupState != StartupState::WAITING_FOR_WIFI)
  {
    return;
  }

  if (_wifiManager.isConnected())
  {
    Trace::log(TraceLevel::INFO, "Startup: WiFi available, initializing OTA");
    _otaManager.setup(DEVICE_NAME, OTA_PASSWORD);
    _connectivityCoordinator.ensureMqttConnected();
    _startupState = StartupState::RUNNING;
    return;
  }

  if (millis() - _startupWaitStart > WIFI_INITIAL_CONNECT_TIMEOUT_MS)
  {
    Trace::log(TraceLevel::WARNING, "Initial WiFi connection timeout; continuing non-blocking");
    _startupState = StartupState::RUNNING;
  }
}
