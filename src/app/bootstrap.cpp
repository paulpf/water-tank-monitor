#include "bootstrap.h"

Bootstrap::Bootstrap()
    : _systemConfig(),
      _app(_wifiManager, _otaManager, _systemConfig, _levelSensor)
{
}

Application &Bootstrap::application()
{
  return _app;
}

