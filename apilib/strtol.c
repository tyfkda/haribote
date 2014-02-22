#include "stdio.h"
#include "string.h"  // toupper

long strtol(char *s, char **endp, int base) {
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
    char c = toupper(*s);
    int h;
    if ('0' <= c && c <= '9')  h = c - '0';
    else if ('A' <= c && c <= 'Z')  h = c - ('A' - 10);
    else break;
    if (h >= base)  break;
    x = x * base + h;
  }

  if (endp != NULL)
    *endp = s;

  return negative ? -x : x;
}
