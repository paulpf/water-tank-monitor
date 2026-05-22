#ifndef CONNECTIVITYCOORDINATOR_H
#define CONNECTIVITYCOORDINATOR_H

#include "imqttconnectioncontrol.h"
#include "iwificonnectivity.h"

class ConnectivityCoordinator
{
public:
  ConnectivityCoordinator(IWifiConnectivity &wifi, IMqttConnectionControl &mqtt)
      : _wifi(wifi), _mqtt(mqtt)
  {
  }

  void handleEvents()
  {
    if (_wifi.consumeDisconnectedEvent())
    {
      _mqtt.forceDisconnect();
    }

    if (_wifi.consumeConnectedEvent())
    {
      _mqtt.requestConnect();
    }
  }

  void ensureMqttConnected()
  {
    handleEvents();
    if (_wifi.isConnected())
    {
      _mqtt.requestConnect();
    }
  }

private:
  IWifiConnectivity &_wifi;
  IMqttConnectionControl &_mqtt;
};

#endif // CONNECTIVITYCOORDINATOR_H
