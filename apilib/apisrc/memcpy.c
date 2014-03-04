#include "string.h"

void* memcpy(void* restrict dst, const void* restrict src, size_t size) {
  char* p = (char*)dst;
  const char* q = (const char*)src;
  for (; size > 0; --size)
    *p++ = *q++;
  return dst;
}
