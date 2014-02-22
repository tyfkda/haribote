#include "stdio.h"
#include "stdarg.h"
#include "api.h"

int printf(const char* format, ...) {
  va_list ap;
  va_start(ap, format);
  char s[1000];  // TODO: Handle buffer overflow.
  int i = vsprintf(s, format, ap);
  api_putstr0(s);
  va_end(ap);
  return i;
}
