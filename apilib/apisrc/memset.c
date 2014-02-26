#include "stdio.h"

void* memset(void* buf, int ch, int n) {
  char* p = (char*)buf;
  for (int i = 0; i < n; ++i)
    *p++ = ch;
  return buf;
}
