#ifndef OTAMANAGER_H
#define OTAMANAGER_H

#include <Arduino.h>
#include <ArduinoOTA.h>
#include "config.h"

class OtaManager
{
public:
  OtaManager();

  // Stores the pointers; both must outlive the manager (string literals).
  void configure(const char *hostname, const char *password);

  // Sets up OTA once, on the first call with WiFi connected, then serves it.
  void loop(bool wifiConnected);

  bool isUpdating() const;
  bool isEnabled() const;
  // True once setup was attempted, even if it failed closed (no password).
  bool isSetupAttempted() const;

private:
  void setup();

  const char *_hostname;
  const char *_password;
  bool _enabled;
  bool _setupAttempted;
  bool _isUpdating;
  unsigned long _lastProgressUpdate;

  void onStart();
  void onEnd();
  void onProgress(unsigned int progress, unsigned int total);
  void onError(ota_error_t error);

  const char *getErrorString(ota_error_t error);
};

#endif // OTAMANAGER_H
