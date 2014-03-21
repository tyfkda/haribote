#include "stdio.h"
#include "stdio_def.h"
#include "apilib.h"

int fseek(FILE *stream, long offset, int whence) {
  api_fseek(stream->handle, offset, whence);
  return 0;  // TODO: Fix.
}
