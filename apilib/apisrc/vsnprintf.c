#include "stdarg.h"
#include "apilib.h"

int vsnprintf(char *str, size_t size, const char *format, va_list ap) {
  // TODO: Check size overflow.
  (void)size;
  return vsprintf(str, format, ap);
}
