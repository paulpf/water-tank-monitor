#ifndef OTAMANAGER_H
#define OTAMANAGER_H

#include <Arduino.h>
#include <ArduinoOTA.h>
#include "config.h"
#include "iotaloopcontrol.h"

class OtaManager : public IOtaLoopControl
{
public:
  OtaManager();

  void setup(const char *hostname, const char *password = nullptr);
  void loop() override;
  bool isUpdating() const override;
  bool isEnabled() const;

private:
  bool _enabled;
  bool _isUpdating;
  unsigned long _lastProgressUpdate;

  void onStart();
  void onEnd();
  void onProgress(unsigned int progress, unsigned int total);
  void onError(ota_error_t error);

  const char *getErrorString(ota_error_t error);
};

#endif // OTAMANAGER_H
