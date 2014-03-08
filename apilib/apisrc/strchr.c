#include "string.h"
#include "stddef.h"

char *strchr(const char *s, int c) {
  for (unsigned char *p = (unsigned char*)s; *s != '\0'; ++p) {
    if (*p == c)
      return (char*)p;
  }
  return NULL;
}
