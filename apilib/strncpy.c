#include "string.h"

char* strncpy(char* dst, const char* src, size_t n) {
  char* orgDst = dst;
  for (; n > 0; --n) {
    if ((*dst++ = *src++) == '\0')
      break;
  }
  return orgDst;
}
