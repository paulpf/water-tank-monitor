#ifndef MQTTCOMMANDS_H
#define MQTTCOMMANDS_H

#include <stdint.h>

// Parsers for MQTT command payloads (not null-terminated). On failure `out`
// is left unchanged.
namespace MqttCommands
{
// Accepts 1-10 decimal digits without leading zero, in
// [CALIBRATION_INTERVAL_MS, UINT32_MAX].
bool parseInterval(const uint8_t *payload, unsigned int length, uint32_t &out);

// Accepts exactly "0" or "1".
bool parseFlag(const uint8_t *payload, unsigned int length, bool &out);
} // namespace MqttCommands

#endif // MQTTCOMMANDS_H
