#include "wifimanager.h"
#include "config.h"
#include "trace.h"

void WifiManager::setup(String ssid, String password, String clientName)
{
  _ssid = ssid;
  _password = password;

  // Configure station mode and hostname before connecting.
  // Hostname helps identify this node in router UI and mDNS environments.
  WiFi.mode(WIFI_STA);
  WiFi.setHostname(clientName.c_str());

  // The SDK reconnects on its own after a loss. Driving reconnects manually
  // (disconnect() + begin() on a short backoff) can abort an association
  // that is still in progress, so the firmware only falls back to begin()
  // after a long outage, see loop().
  WiFi.setAutoReconnect(true);
  WiFi.begin(_ssid.c_str(), _password.c_str());

  // Default modem sleep delays responses (ping, OTA UDP invite, MQTT) between
  // beacon intervals - this device is mains-powered, so trade power for latency.
  // Set after begin(): applying it before the connection handshake has been
  // observed to interfere with association on some ESP8266 core versions.
  WiFi.setSleepMode(WIFI_NONE_SLEEP);
  _lastBeginMs = millis();

  Trace::log(TraceLevel::DEBUG, "WiFi setup complete.");
}

void WifiManager::loop()
{
  const bool connectedNow = (WiFi.status() == WL_CONNECTED);

  if (connectedNow && !_connected)
  {
    const IPAddress ip = WiFi.localIP();
    Trace::logf(TraceLevel::INFO, "WiFi connected, IP: %u.%u.%u.%u", ip[0],
                ip[1], ip[2], ip[3]);
    _connected = true;
  }
  else if (!connectedNow && _connected)
  {
    Trace::log(TraceLevel::INFO, "WiFi disconnected, waiting for auto-reconnect...");
    _connected = false;
    _lastBeginMs = millis();
  }

  if (!_connected && millis() - _lastBeginMs >= WIFI_FALLBACK_BEGIN_MS)
  {
    Trace::log(TraceLevel::WARNING, "WiFi still disconnected, restarting connection");
    WiFi.begin(_ssid.c_str(), _password.c_str());
    _lastBeginMs = millis();
  }
}
