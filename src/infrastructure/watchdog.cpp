#include "watchdog.h"
#include <Arduino.h>

uint32_t Watchdog::_timeoutMs = 30000;
volatile uint32_t Watchdog::_lastFeedMs = 0;

void Watchdog::setup(uint32_t timeoutMs)
{
  _timeoutMs = timeoutMs;
  _lastFeedMs = millis();
  _ticker.attach_ms(1000, checkTimeout);
}

void Watchdog::feed()
{
  _lastFeedMs = millis();
}

void Watchdog::checkTimeout()
{
  if (millis() - _lastFeedMs > _timeoutMs)
  {
    ESP.restart();
  }
}
