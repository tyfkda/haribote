#include "string.h"

void* memset(void* buf, int ch, size_t n) {
  char* p = (char*)buf;
  for (size_t i = 0; i < n; ++i)
    *p++ = ch;
  return buf;
}
