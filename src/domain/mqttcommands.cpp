#include "mqttcommands.h"
#include "config.h"

namespace MqttCommands
{

bool parseInterval(const uint8_t *payload, unsigned int length, uint32_t &out)
{
  if (length == 0 || length > 10 || payload[0] == '0')
  {
    return false;
  }

  uint64_t value = 0;
  for (unsigned int i = 0; i < length; i++)
  {
    if (payload[i] < '0' || payload[i] > '9')
    {
      return false;
    }
    value = value * 10 + (payload[i] - '0');
  }

  if (value < CALIBRATION_INTERVAL_MS || value > UINT32_MAX)
  {
    return false;
  }
  out = static_cast<uint32_t>(value);
  return true;
}

bool parseFlag(const uint8_t *payload, unsigned int length, bool &out)
{
  if (length != 1 || (payload[0] != '0' && payload[0] != '1'))
  {
    return false;
  }
  out = payload[0] == '1';
  return true;
}

} // namespace MqttCommands
