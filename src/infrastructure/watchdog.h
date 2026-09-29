#ifndef WATCHDOG_H
#define WATCHDOG_H

#include <stdint.h>
#include <Ticker.h>

// Software watchdog: resets the device if feed() isn't called within
// timeoutMs. On ESP8266, Ticker is an SDK software timer (os_timer), not a
// hardware interrupt: it only fires while the code yields (delay(), yield(),
// blocking network calls). So this catches hangs inside yielding calls;
// hangs that never yield are caught by the core's own soft/hardware WDT.
// feed() is static so long blocking operations (e.g. OTA transfer) can feed
// the watchdog without holding a reference to the instance.
class Watchdog
{
public:
  void setup(uint32_t timeoutMs);
  static void feed();

private:
  static void checkTimeout();

  Ticker _ticker;
  static uint32_t _timeoutMs;
  static volatile uint32_t _lastFeedMs;
};

#endif // WATCHDOG_H
