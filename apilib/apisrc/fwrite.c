#include "stdio.h"
#include "stdio_def.h"
#include "apilib.h"

size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream) {
  size_t totalSize = size * nmemb;
  int writeSize = api_fwrite(ptr, totalSize, stream->handle);
  return writeSize / nmemb;
}
