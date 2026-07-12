#include "bootstrap.h"

Bootstrap::Bootstrap()
    : _systemConfig(),
      _connectivityCoordinator(_wifiManager, _mqttManager),
      _app(_wifiManager, _otaManager, _systemConfig, _levelSensor,
           _mqttManager, _connectivityCoordinator, _watchdog)
{
}

Application &Bootstrap::application()
{
  return _app;
}
