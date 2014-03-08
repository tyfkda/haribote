#include "string.h"
#include "stddef.h"

void *memchr(const void *s, int c, size_t n) {
  for (unsigned char *p = (unsigned char*)s; n > 0; ++p, --n) {
    if (*p == c)
      return p;
  }
  return NULL;
}
