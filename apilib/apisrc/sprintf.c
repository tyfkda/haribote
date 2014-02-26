#include "stdio.h"
#include "stdarg.h"

int sprintf(char *str, const char *format, ...) {
  va_list ap;
  va_start(ap, format);
  int i = vsprintf(str, format, ap);
  va_end(ap);
  return i;
}
