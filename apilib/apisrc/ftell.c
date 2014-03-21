#include "stdio.h"
#include "stdio_def.h"
#include "apilib.h"

long ftell(FILE *stream) {
  return api_fsize(stream->handle, 1);
}
