#include "trace.h"
#include <stdarg.h>
#include <stdio.h>

static const char *prefixFor(TraceLevel level)
{
  switch (level)
  {
  case TraceLevel::TRACE:
    return "[TRACE] ";
  case TraceLevel::DEBUG:
    return "[DEBUG] ";
  case TraceLevel::INFO:
    return "[INFO] ";
  case TraceLevel::WARNING:
    return "[WARNING] ";
  case TraceLevel::ERROR:
    return "[ERROR] ";
  default:
    return "[LOG] ";
  }
}

void Trace::log(TraceLevel level, const String &message)
{
  logf(level, "%s", message.c_str());
}

void Trace::logf(TraceLevel level, const char *format, ...)
{
  if (!shouldLog(level))
  {
    return;
  }

  // Fixed-size stack buffer keeps memory behavior predictable on MCU.
  // Messages longer than buffer are truncated by vsnprintf.
  char messageBuffer[160];
  va_list args;
  va_start(args, format);
  vsnprintf(messageBuffer, sizeof(messageBuffer), format, args);
  va_end(args);

  Serial.print(prefixFor(level));
  Serial.println(messageBuffer);
}
