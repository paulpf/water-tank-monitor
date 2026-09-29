#ifndef WIFIMANAGER_H
#define WIFIMANAGER_H

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "iwificonnectivity.h"

class WifiManager : public IWifiConnectivity
{
public:
  void setup(String ssid, String password, String clientName);
  bool loop();
  bool isConnected() const override
  {
    return _wifiState == WIFI_CONNECTED;
  }
  bool consumeConnectedEvent() override;
  bool consumeDisconnectedEvent() override;

private:
  void manageConnection();

  String _ssid;
  String _password;
  String _clientName;
  unsigned long _lastAttemptTime = 0;
  uint32_t _reconnectDelayMs = 0;
  enum WifiState
  {
    WIFI_DISCONNECTED,
    WIFI_CONNECTING,
    WIFI_CONNECTED
  };
  WifiState _wifiState = WIFI_DISCONNECTED;
  unsigned long _wifiConnectStartTime = 0;
  uint8_t _reconnectAttempt = 0;
  bool _connectedEventPending = false;
  bool _disconnectedEventPending = false;
};

#endif // WIFIMANAGER_H
