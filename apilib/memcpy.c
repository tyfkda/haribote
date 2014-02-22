#include "stdio.h"

void* memcpy(void* dst, const void* src, int size) {
  char* p = (char*)dst;
  const char* q = (const char*)src;
  for (; size > 0; --size)
    *p++ = *q++;
  return dst;
}
