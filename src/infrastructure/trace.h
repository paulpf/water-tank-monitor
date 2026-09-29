#ifndef TRACE_H
#define TRACE_H

#include <Arduino.h>
#include "config.h"

/**
 * Represents different trace levels for logging
 */
enum class TraceLevel
{
  TRACE, // Detailed trace information
  DEBUG, // Debug information for development
  INFO,  // General information messages
  WARNING, // Warning messages for recoverable issues
  ERROR, // Error messages indicating issues
  NONE   // No logging
};

class Trace
{
public:
  static void log(TraceLevel level, const String &message);
  static void logf(TraceLevel level, const char *format, ...)
      __attribute__((format(printf, 2, 3)));

private:
  // Check if the message should be logged based on the configured level
  static bool shouldLog(TraceLevel level)
  {
    return static_cast<int>(level) >= static_cast<int>(TRACE_LEVEL);
  }
};

#endif // TRACE_H
