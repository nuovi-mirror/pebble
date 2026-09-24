#include "snprint.h"

#include <stdarg.h>
#include <stdio.h>

int snprint(char* restrict str, unsigned long size, const char* restrict format,
            ...) {
  va_list args;
  va_start(args, format);

  int result = vsnprintf(str, size, format, args);

  va_end(args);
  return result;
}
