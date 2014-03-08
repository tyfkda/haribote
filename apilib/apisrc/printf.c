#include "stdio.h"
#include "stdarg.h"

int printf(const char* format, ...) {
  va_list ap;
  va_start(ap, format);
  int i = vfprintf(stdout, format, ap);
  va_end(ap);
  return i;
}
