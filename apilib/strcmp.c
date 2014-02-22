#include "stdio.h"

int strcmp(const char* s1, const char* s2) {
  for (unsigned char *u1 = (unsigned char*)s1, *u2 = (unsigned char*)s2;
       ; ++u1, ++u2) {
    int d = *u1 - *u2;
    if (d != 0)
      return d;
    if (*u1 == '\0')
      return 0;
  }
}
