#include "stddef.h"
#include "stdlib.h"

long strtol(const char *s, char **endp, int base) {
  char negative = FALSE;
  switch (*s) {
  case '+':  ++s; break;
  case '-':  negative = TRUE; ++s; break;
  default:  break;
  }

  if (base == 0) {
    if (*s == '0') {
      ++s;
      if (*s == 'x') {
        ++s;
        base = 16;
      } else
        base = 8;
    } else
      base = 10;
  }

  int x = 0;
  for (;; ++s) {
    char c = *s;
    int h;
    if ('0' <= c && c <= '9')  h = c - '0';
    else if ('A' <= c && c <= 'Z')  h = c - ('A' - 10);
    else if ('a' <= c && c <= 'z')  h = c - ('a' - 10);
    else break;
    if (h >= base)  break;
    x = x * base + h;
  }

  if (endp != NULL)
    *endp = (char*)s;

  return negative ? -x : x;
}
