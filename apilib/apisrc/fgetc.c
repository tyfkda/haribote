#include "stdio.h"
#include "stdio_def.h"
#include "apilib.h"

int fgetc(FILE *stream) {
  unsigned char c;
  int readSize = api_fread(&c, 1, stream->handle);
  if (readSize < 1)
    return EOF;
  return c;
}
