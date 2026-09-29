#ifndef WIFIMANAGER_H
#define WIFIMANAGER_H

#include <Arduino.h>
#include <ESP8266WiFi.h>

class WifiManager
{
public:
  void setup(String ssid, String password, String clientName);
  void loop();
  bool isConnected() const
  {
    return _wifiState == WIFI_CONNECTED;
  }

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
};

#endif // WIFIMANAGER_H
