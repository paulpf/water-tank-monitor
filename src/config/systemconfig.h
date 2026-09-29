#ifndef SYSTEMCONFIG_H
#define SYSTEMCONFIG_H

#include <stdint.h>
#include "config.h"

struct SystemConfig
{
  // Watchdog timeout to reset system on lockup
  uint32_t watchdogTimeoutMs = WATCHDOG_TIMEOUT;

  // Sensor read interval, runtime configurable via MQTT_TOPIC_READ_INTERVAL_SET
  uint32_t sensorReadIntervalMs = SENSOR_READ_INTERVAL_MS;

  // MQTT publish interval, runtime configurable via MQTT_TOPIC_PUBLISH_INTERVAL_SET
  // Decoupled from sensorReadIntervalMs: reading can run fast (e.g. for calibration)
  // while publishing stays at a slower, network-friendly cadence, or vice versa.
  uint32_t publishIntervalMs = MQTT_PUBLISH_INTERVAL_MS;

  // When true, both read and publish cadence are forced to CALIBRATION_INTERVAL_MS,
  // regardless of sensorReadIntervalMs/publishIntervalMs. Toggle via
  // MQTT_TOPIC_CALIBRATION_MODE_SET; leaves the configured intervals untouched.
  bool calibrationMode = false;

  uint32_t effectiveReadIntervalMs() const
  {
    return calibrationMode ? CALIBRATION_INTERVAL_MS : sensorReadIntervalMs;
  }

  uint32_t effectivePublishIntervalMs() const
  {
    return calibrationMode ? CALIBRATION_INTERVAL_MS : publishIntervalMs;
  }
};

#endif // SYSTEMCONFIG_H
