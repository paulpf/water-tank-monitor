#include "application.h"
#include "trace.h"
#include "tanklevel.h"
#include "config.h"
#include <ESP8266WiFi.h>
#include <cstring>
#include <cstdlib>

#include "../../../_config/MqttConfig.h"
#include "../../../_secrets/MqttSecret.h"
#include "../../../_secrets/OtaSecret.h"
#include "../../../_secrets/WifiSecret.h"

Application::Application(WifiManager &wifiManager, OtaManager &otaManager,
                         SystemConfig &systemConfig, ILevelSensor &levelSensor,
                         MqttManager &mqttManager,
                         ConnectivityCoordinator &connectivityCoordinator,
                         Watchdog &watchdog)
    : _wifiManager(wifiManager), _otaManager(otaManager),
      _systemConfig(systemConfig), _levelSensor(levelSensor),
      _mqttManager(mqttManager),
      _connectivityCoordinator(connectivityCoordinator),
      _watchdog(watchdog),
      _lastStatusPrint(0), _lastSensorRead(0), _lastPublish(0), _lastRssiPublish(0), _startupWaitStart(0),
      _startupState(StartupState::WAITING_FOR_WIFI),
      _mqttWasConnected(false),
      _otaSetupDone(false),
      _lastLevel{0.0f, 0.0f, 0.0f, 13.0f, 408.0f, 0.0f},
      _lastSensorValid(false)
{
}

void Application::setup()
{
  Trace::log(TraceLevel::INFO, "Application setup started");

  _watchdog.setup(_systemConfig.watchdogTimeoutMs);

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
  _mqttManager.setCallback([this](char *topic, uint8_t *payload, unsigned int length) {
    handleMqttMessage(topic, payload, length);
  });
  _mqttManager.subscribe(MQTT_TOPIC_COMMAND_RESET);
  _mqttManager.subscribe(MQTT_TOPIC_READ_INTERVAL_SET);
  _mqttManager.subscribe(MQTT_TOPIC_PUBLISH_INTERVAL_SET);
  _mqttManager.subscribe(MQTT_TOPIC_CALIBRATION_MODE_SET);

  _startupWaitStart = millis();
  _startupState = StartupState::WAITING_FOR_WIFI;
  Trace::log(TraceLevel::INFO, "Startup is non-blocking, waiting for WiFi in main loop");

  Trace::log(TraceLevel::INFO, "Application setup complete");
  _lastStatusPrint = millis();
}

void Application::loop()
{
  unsigned long currentTime = millis();

  _watchdog.feed();

  handleStartup();

  // Decoupled from the one-shot startup state machine: guarantees OTA gets
  // initialized exactly once as soon as WiFi is up, no matter how long the
  // initial connection takes (handleStartup() gives up waiting after
  // WIFI_INITIAL_CONNECT_TIMEOUT_MS and would otherwise skip this forever).
  if (!_otaSetupDone && _wifiManager.isConnected())
  {
    _otaManager.setup(DEVICE_NAME, OTA_PASSWORD);
    _otaSetupDone = true;
  }

  _wifiManager.loop();
  _otaManager.loop();
  _connectivityCoordinator.handleEvents();
  _mqttManager.loop();

  bool mqttConnectedNow = _mqttManager.isConnected();
  if (mqttConnectedNow && !_mqttWasConnected)
  {
    Trace::log(TraceLevel::INFO, "MQTT connected - publishing initial values");
    char ipBuf[16];
    WiFi.localIP().toString().toCharArray(ipBuf, sizeof(ipBuf));
    _mqttManager.publishRetained(MQTT_TOPIC_IP, ipBuf);

    char intervalBuf[16];
    snprintf(intervalBuf, sizeof(intervalBuf), "%lu", _systemConfig.sensorReadIntervalMs);
    _mqttManager.publishRetained(MQTT_TOPIC_READ_INTERVAL_MS, intervalBuf);

    snprintf(intervalBuf, sizeof(intervalBuf), "%lu", _systemConfig.publishIntervalMs);
    _mqttManager.publishRetained(MQTT_TOPIC_PUBLISH_INTERVAL_MS, intervalBuf);

    _mqttManager.publishRetained(MQTT_TOPIC_CALIBRATION_MODE, _systemConfig.calibrationMode ? "1" : "0");

    // Seed retained "0" so the command datapoint exists in MQTT tools (e.g. ioBroker)
    // before the user ever writes to it.
    _mqttManager.publishRetained(MQTT_TOPIC_COMMAND_RESET, "0");

    readSensor();
    publishLevel();
    _lastSensorRead = currentTime;
    _lastPublish = currentTime;
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

  unsigned long effectiveReadIntervalMs = _systemConfig.calibrationMode
                                               ? CALIBRATION_INTERVAL_MS
                                               : _systemConfig.sensorReadIntervalMs;
  unsigned long effectivePublishIntervalMs = _systemConfig.calibrationMode
                                                  ? CALIBRATION_INTERVAL_MS
                                                  : _systemConfig.publishIntervalMs;

  if (currentTime - _lastSensorRead >= effectiveReadIntervalMs)
  {
    _lastSensorRead = currentTime;
    readSensor();
  }

  if (_mqttManager.isConnected() &&
      currentTime - _lastPublish >= effectivePublishIntervalMs)
  {
    _lastPublish = currentTime;
    publishLevel();
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

void Application::readSensor()
{
  _lastLevel = _levelSensor.read();
  _lastSensorValid = _lastLevel.isValid();

  if (!_lastSensorValid)
  {
    char buf[64];
    snprintf(buf, sizeof(buf), "Sensor out of range: %.2f mA - check wiring", _lastLevel.currentMa);
    Trace::log(TraceLevel::WARNING, buf);
  }
  else
  {
    char buf[96];
    snprintf(buf, sizeof(buf), "Tank: %.1f%% | %.1f cm | %.0f L | %.2f mA",
             _lastLevel.levelPercent, _lastLevel.heightCm, _lastLevel.volumeLiters, _lastLevel.currentMa);
    Trace::log(TraceLevel::INFO, buf);
  }
}

void Application::publishLevel()
{
  if (!_mqttManager.isConnected())
  {
    return;
  }

  char payload[16];
  snprintf(payload, sizeof(payload), "%.2f", _lastLevel.currentMa);
  _mqttManager.publishRetained(MQTT_TOPIC_CURRENT_MA, payload);

  snprintf(payload, sizeof(payload), "%.4f", _lastLevel.voltageV);
  _mqttManager.publishRetained(MQTT_TOPIC_VOLTAGE_V, payload);

  _mqttManager.publishRetained(MQTT_TOPIC_VALID, _lastSensorValid ? "true" : "false");

  if (_lastSensorValid)
  {
    snprintf(payload, sizeof(payload), "%.1f", _lastLevel.levelPercent);
    _mqttManager.publishRetained(MQTT_TOPIC_LEVEL_PERCENT, payload);

    snprintf(payload, sizeof(payload), "%.1f", _lastLevel.heightCm);
    _mqttManager.publishRetained(MQTT_TOPIC_HEIGHT_CM, payload);

    snprintf(payload, sizeof(payload), "%.0f", _lastLevel.volumeLiters);
    _mqttManager.publishRetained(MQTT_TOPIC_VOLUME_LITERS, payload);

    snprintf(payload, sizeof(payload), "%.0f", _lastLevel.overflowLiters);
    _mqttManager.publishRetained(MQTT_TOPIC_VOLUME_OVERFLOW_L, payload);
  }

  publishHealth();
}

void Application::publishHealth()
{
  char payload[192];
  snprintf(payload, sizeof(payload),
           "{\"sensorReady\":%s,\"sensorValid\":%s,\"currentMa\":%.2f,"
           "\"wifiRssi\":%d,\"uptimeMs\":%lu,\"freeHeap\":%u,"
           "\"otaReady\":%s,\"otaEnabled\":%s,\"otaUpdating\":%s}",
           _levelSensor.isReady() ? "true" : "false",
           _lastSensorValid ? "true" : "false",
           _lastLevel.currentMa,
           WiFi.RSSI(),
           millis(),
           ESP.getFreeHeap(),
           _otaSetupDone ? "true" : "false",
           _otaManager.isEnabled() ? "true" : "false",
           _otaManager.isUpdating() ? "true" : "false");
  _mqttManager.publishRetained(MQTT_TOPIC_HEALTH, payload);
}

void Application::handleMqttMessage(char *topic, uint8_t *payload, unsigned int length)
{
  if (strcmp(topic, MQTT_TOPIC_COMMAND_RESET) == 0)
  {
    // Require an explicit "1" so a retained/stale message (e.g. redelivered on
    // reconnect) can never re-trigger a restart and cause a boot loop.
    if (length == 1 && payload[0] == '1')
    {
      Trace::log(TraceLevel::WARNING, "MQTT reset command received - restarting");
      // Clear the retained flag before restarting, otherwise the broker will
      // redeliver "1" on the next (re)subscribe and restart again immediately.
      _mqttManager.publishRetained(MQTT_TOPIC_COMMAND_RESET, "0");
      delay(100);
      ESP.restart();
    }
    return;
  }

  if (strcmp(topic, MQTT_TOPIC_READ_INTERVAL_SET) == 0)
  {
    char buf[16];
    unsigned int len = length < sizeof(buf) - 1 ? length : sizeof(buf) - 1;
    memcpy(buf, payload, len);
    buf[len] = '\0';

    unsigned long newInterval = strtoul(buf, nullptr, 10);
    if (newInterval > 0)
    {
      _systemConfig.sensorReadIntervalMs = newInterval;
      Trace::log(TraceLevel::INFO, "Sensor read interval updated via MQTT");
      _mqttManager.publishRetained(MQTT_TOPIC_READ_INTERVAL_MS, buf);
    }
    return;
  }

  if (strcmp(topic, MQTT_TOPIC_PUBLISH_INTERVAL_SET) == 0)
  {
    char buf[16];
    unsigned int len = length < sizeof(buf) - 1 ? length : sizeof(buf) - 1;
    memcpy(buf, payload, len);
    buf[len] = '\0';

    unsigned long newInterval = strtoul(buf, nullptr, 10);
    if (newInterval > 0)
    {
      _systemConfig.publishIntervalMs = newInterval;
      Trace::log(TraceLevel::INFO, "Publish interval updated via MQTT");
      _mqttManager.publishRetained(MQTT_TOPIC_PUBLISH_INTERVAL_MS, buf);
    }
    return;
  }

  if (strcmp(topic, MQTT_TOPIC_CALIBRATION_MODE_SET) == 0)
  {
    if (length == 1 && (payload[0] == '1' || payload[0] == '0'))
    {
      _systemConfig.calibrationMode = (payload[0] == '1');
      Trace::log(TraceLevel::INFO, _systemConfig.calibrationMode
                                        ? "Calibration mode ON (500ms read+publish)"
                                        : "Calibration mode OFF");
      _mqttManager.publishRetained(MQTT_TOPIC_CALIBRATION_MODE,
                                    _systemConfig.calibrationMode ? "1" : "0");
    }
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
    Trace::log(TraceLevel::INFO, "Startup: WiFi available");
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
