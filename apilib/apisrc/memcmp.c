#include "string.h"

int memcmp(const void *s1, const void *s2, size_t n) {
  for (unsigned char *u1 = (unsigned char*)s1, *u2 = (unsigned char*)s2;
       n > 0; ++u1, ++u2, --n) {
    int d = *u1 - *u2;
    if (d != 0)
      return d;
  }
  return 0;
}
