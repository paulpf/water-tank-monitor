#include "bootstrap.h"

Bootstrap::Bootstrap()
    : _systemConfig(),
      _app(_wifiManager, _otaManager, _systemConfig, _levelSensor,
           _mqttManager, _watchdog)
{
}

Application &Bootstrap::application()
{
  return _app;
}
