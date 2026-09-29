#ifndef INTERVALTIMER_H
#define INTERVALTIMER_H

#include <stdint.h>

// Wrap-safe periodic timer for millis() timestamps. uint32_t on purpose:
// millis() is 32 bit on the ESP8266, while unsigned long is 64 bit in the
// native test build.
struct IntervalTimer
{
  uint32_t last = 0;

  // Missed intervals are not caught up: after a long pause it fires once.
  bool due(uint32_t now, uint32_t interval)
  {
    if (static_cast<uint32_t>(now - last) < interval)
    {
      return false;
    }
    last = now;
    return true;
  }

  void reset(uint32_t now) { last = now; }
};

#endif // INTERVALTIMER_H
