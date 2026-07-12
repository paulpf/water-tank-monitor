#ifndef WATCHDOG_H
#define WATCHDOG_H

#include <stdint.h>
#include <Ticker.h>

// Software watchdog: resets the device if feed() isn't called within
// timeoutMs. Ticker runs on a hardware timer interrupt, so it still fires
// even if the main loop() is stuck (as long as interrupts aren't disabled).
class Watchdog
{
public:
  void setup(uint32_t timeoutMs);
  void feed();

private:
  static void checkTimeout();

  Ticker _ticker;
  static uint32_t _timeoutMs;
  static volatile uint32_t _lastFeedMs;
};

#endif // WATCHDOG_H
