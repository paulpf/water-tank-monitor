#ifndef SYSTEMCONFIG_H
#define SYSTEMCONFIG_H

#include "config.h"

struct SystemConfig
{
  // Watchdog timeout to reset system on lockup
  unsigned long watchdogTimeoutMs = WATCHDOG_TIMEOUT;
  
  // Main loop interval for long-running tasks (e.g., periodic telemetry)
  unsigned long longIntervalMs = LONG_INTERVAL;
  
  // Main loop interval for medium-frequency tasks
  unsigned long middleIntervalMs = MIDDLE_INTERVAL;
  
  // Main loop interval for high-frequency tasks
  unsigned long shortIntervalMs = SHORT_INTERVAL;

  // Sensor read interval, runtime configurable via MQTT_TOPIC_READ_INTERVAL_SET
  unsigned long sensorReadIntervalMs = SENSOR_READ_INTERVAL_MS;

  // MQTT publish interval, runtime configurable via MQTT_TOPIC_PUBLISH_INTERVAL_SET
  // Decoupled from sensorReadIntervalMs: reading can run fast (e.g. for calibration)
  // while publishing stays at a slower, network-friendly cadence, or vice versa.
  unsigned long publishIntervalMs = MQTT_PUBLISH_INTERVAL_MS;

  // When true, both read and publish cadence are forced to CALIBRATION_INTERVAL_MS,
  // regardless of sensorReadIntervalMs/publishIntervalMs. Toggle via
  // MQTT_TOPIC_CALIBRATION_MODE_SET; leaves the configured intervals untouched.
  bool calibrationMode = false;
};

#endif // SYSTEMCONFIG_H
