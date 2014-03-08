#include "stdarg.h"
#include "apilib.h"

int vfprintf(struct FILE* stream, const char *format, va_list ap) {
  // TODO: Stop using buffer.
  char str[256];
  int i = vsnprintf(str, sizeof(str), format, ap);
  // TODO: Use stream to output string.
  (void)stream;
  api_putstr0(str);
  return i;
}
