#include "stdio_def.h"
#include "stdarg.h"
#include "apilib.h"

int fprintf(FILE* stream, const char* format, ...) {
  va_list ap;
  va_start(ap, format);
  char s[1000];  // TODO: Handle buffer overflow.
  int i = vsprintf(s, format, ap);
  va_end(ap);
  api_fwrite(s, i, stream->handle);
  return i;
}
