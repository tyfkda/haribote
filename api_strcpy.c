#include "stdio.h"

char* strcpy(char* dst, const char* src) {
  char* orgDst = dst;
  for (;;) {
    if ((*dst++ = *src++) == '\0')
      break;
  }
  return orgDst;
}
