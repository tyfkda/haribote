#include "string.h"

void* memmove(void* dst, const void* src, size_t size) {
  char* p = (char*)dst;
  const char* q = (const char*)src;
  if (dst < src) {
    for (; size > 0; --size)
      *p++ = *q++;
  } else {
    p += size;
    q += size;
    for (; size > 0; --size)
      *(--p) = *(--q);
  }
  return dst;
}
