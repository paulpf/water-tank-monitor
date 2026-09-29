#include "application.h"
#include "config.h"
#include "mqttcommands.h"
#include "tanklevel.h"
#include "trace.h"
#include <ESP8266WiFi.h>
#include <cstring>

#include "MqttConfig.h"
#include "MqttSecret.h"
#include "OtaSecret.h"
#include "WifiSecret.h"

Application::Application(WifiManager &wifiManager, OtaManager &otaManager,
                         SystemConfig &systemConfig, LevelSensor &levelSensor,
                         MqttManager &mqttManager, Watchdog &watchdog)
    : _wifiManager(wifiManager), _otaManager(otaManager),
      _systemConfig(systemConfig), _levelSensor(levelSensor),
      _mqttManager(mqttManager), _watchdog(watchdog),
      _lastLevel(TankLevel::notReady()),
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
  _otaManager.configure(DEVICE_NAME, OTA_PASSWORD);
  _mqttManager.setup(MQTT_SERVER_IP, MQTT_SERVER_PORT, MQTT_USER, MQTT_PWD, DEVICE_NAME);
  _mqttManager.setCallback([this](char *topic, uint8_t *payload, unsigned int length) {
    handleMqttMessage(topic, payload, length);
  });
  _mqttManager.subscribe(MQTT_TOPIC_COMMAND_RESET);
  _mqttManager.subscribe(MQTT_TOPIC_READ_INTERVAL_SET);
  _mqttManager.subscribe(MQTT_TOPIC_PUBLISH_INTERVAL_SET);
  _mqttManager.subscribe(MQTT_TOPIC_CALIBRATION_MODE_SET);

  Trace::log(TraceLevel::INFO, "Application setup complete");
  _statusTimer.reset(millis());
}

void Application::loop()
{
  // Read once: every step below must use the same timestamp (see onMqttConnected).
  const uint32_t now = millis();

  _watchdog.feed();

  _wifiManager.loop();
  const bool wifiConnected = _wifiManager.isConnected();
  _otaManager.loop(wifiConnected);
  if (_mqttManager.loop(wifiConnected))
  {
    onMqttConnected(now);
  }

  publishRssiIfDue(now);
  readSensorIfDue(now);
  publishLevelIfDue(now);
  printStatusIfDue(now);
}

// `now` must be the loop's start time, not millis() read here: connect() blocked
// before this call, and a later timestamp would make the read/publish timers
// underflow and fire a second time in the same iteration.
void Application::onMqttConnected(uint32_t now)
{
  Trace::log(TraceLevel::INFO, "MQTT connected - publishing initial values");
  char ipBuf[16];
  WiFi.localIP().toString().toCharArray(ipBuf, sizeof(ipBuf));
  _mqttManager.publishRetained(MQTT_TOPIC_IP, ipBuf);

  publishInterval(MQTT_TOPIC_READ_INTERVAL_MS, _systemConfig.sensorReadIntervalMs);
  publishInterval(MQTT_TOPIC_PUBLISH_INTERVAL_MS, _systemConfig.publishIntervalMs);
  _mqttManager.publishRetained(MQTT_TOPIC_CALIBRATION_MODE, _systemConfig.calibrationMode ? "1" : "0");

  // Seed retained "0" so the command datapoint exists in MQTT tools (e.g. ioBroker)
  // before the user ever writes to it.
  _mqttManager.publishRetained(MQTT_TOPIC_COMMAND_RESET, "0");

  readSensor();
  publishLevel();
  _readTimer.reset(now);
  _publishTimer.reset(now);
}

void Application::publishRssiIfDue(uint32_t now)
{
  // Connection check first: the timer must not be consumed while offline.
  if (_mqttManager.isConnected() && _rssiTimer.due(now, MQTT_RSSI_INTERVAL_MS))
  {
    char payload[8];
    snprintf(payload, sizeof(payload), "%d", WiFi.RSSI());
    _mqttManager.publish(MQTT_TOPIC_RSSI, payload);
  }
}

void Application::readSensorIfDue(uint32_t now)
{
  if (_readTimer.due(now, _systemConfig.effectiveReadIntervalMs()))
  {
    readSensor();
  }
}

void Application::publishLevelIfDue(uint32_t now)
{
  if (_mqttManager.isConnected() && _publishTimer.due(now, _systemConfig.effectivePublishIntervalMs()))
  {
    publishLevel();
  }
}

void Application::printStatusIfDue(uint32_t now)
{
  if (!_statusTimer.due(now, STATUS_PRINT_INTERVAL_MS))
  {
    return;
  }

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

void Application::readSensor()
{
  _lastLevel = _levelSensor.read();
  _lastSensorValid = _lastLevel.isValid();

  if (!_lastSensorValid)
  {
    Trace::logf(TraceLevel::WARNING, "Sensor out of range: %.2f mA - check wiring", _lastLevel.currentMa);
  }
  else
  {
    Trace::logf(TraceLevel::INFO, "Tank: %.1f%% | %.1f cm | %.0f L | %.2f mA",
                _lastLevel.levelPercent, _lastLevel.heightCm, _lastLevel.volumeLiters, _lastLevel.currentMa);
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
  // Worst case is 171 characters. PubSubClient's default 256-byte packet
  // buffer leaves 217 for the payload with this topic; check when adding fields.
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
           _otaManager.isSetupAttempted() ? "true" : "false",
           _otaManager.isEnabled() ? "true" : "false",
           _otaManager.isUpdating() ? "true" : "false");
  _mqttManager.publishRetained(MQTT_TOPIC_HEALTH, payload);
}

void Application::publishInterval(const char *topic, uint32_t intervalMs)
{
  // uint32_t is unsigned int on xtensa and unsigned long elsewhere; the cast
  // keeps %lu correct on both.
  char buf[16];
  snprintf(buf, sizeof(buf), "%lu", static_cast<unsigned long>(intervalMs));
  _mqttManager.publishRetained(topic, buf);
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
    applyInterval(payload, length, _systemConfig.sensorReadIntervalMs,
                  MQTT_TOPIC_READ_INTERVAL_MS, "Sensor read interval updated via MQTT");
    return;
  }

  if (strcmp(topic, MQTT_TOPIC_PUBLISH_INTERVAL_SET) == 0)
  {
    applyInterval(payload, length, _systemConfig.publishIntervalMs,
                  MQTT_TOPIC_PUBLISH_INTERVAL_MS, "Publish interval updated via MQTT");
    return;
  }

  if (strcmp(topic, MQTT_TOPIC_CALIBRATION_MODE_SET) == 0)
  {
    bool enabled;
    if (MqttCommands::parseFlag(payload, length, enabled))
    {
      _systemConfig.calibrationMode = enabled;
      Trace::log(TraceLevel::INFO, enabled ? "Calibration mode ON (500ms read+publish)"
                                           : "Calibration mode OFF");
      _mqttManager.publishRetained(MQTT_TOPIC_CALIBRATION_MODE, enabled ? "1" : "0");
    }
  }
}

void Application::applyInterval(const uint8_t *payload, unsigned int length, uint32_t &target,
                                const char *stateTopic, const char *logMessage)
{
  uint32_t intervalMs;
  if (!MqttCommands::parseInterval(payload, length, intervalMs))
  {
    Trace::logf(TraceLevel::WARNING, "Ignoring invalid interval for %s (digits only, >= %lu ms)",
                stateTopic, static_cast<unsigned long>(CALIBRATION_INTERVAL_MS));
    return;
  }

  target = intervalMs;
  Trace::log(TraceLevel::INFO, logMessage);
  publishInterval(stateTopic, intervalMs);
}
