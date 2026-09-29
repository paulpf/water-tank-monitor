#include "application.h"
#include "config.h"

WifiManager wifiManager;
OtaManager otaManager;
SystemConfig systemConfig;
LevelSensor levelSensor;
MqttManager mqttManager;
Watchdog watchdog;
Application app(wifiManager, otaManager, systemConfig, levelSensor, mqttManager, watchdog);

void setup()
{
  Serial.begin(SERIAL_BAUD_RATE);
  delay(SERIAL_STARTUP_DELAY_MS);

  app.setup();
}

void loop()
{
  app.loop();
}
