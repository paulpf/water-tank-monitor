#ifndef WIFIMANAGER_H
#define WIFIMANAGER_H

#include <Arduino.h>
#include <ESP8266WiFi.h>

class WifiManager
{
public:
  void setup(String ssid, String password, String clientName);
  // Tracks connection changes for logging; reconnecting is left to the SDK.
  void loop();
  bool isConnected() const
  {
    return _connected;
  }

private:
  String _ssid;
  String _password;
  bool _connected = false;
  unsigned long _lastBeginMs = 0;
};

#endif // WIFIMANAGER_H
