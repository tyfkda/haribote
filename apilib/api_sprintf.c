#include "stdio.h"

static char* uint2num(char *s, unsigned int x, int base, const char* table,
                      char padding, int keta) {
  *(--s) = '\0';
  do {
    *(--s) = table[x % base];
    x /= base;
    --keta;
  } while (x > 0);
  for (; keta > 0; --keta)
    *(--s) = padding;
  return s;
}

static char* int2num(char *s, int x, int base, const char* table,
                     char padding, int keta) {
  int negative = FALSE;
  if (x < 0) {
    x = -x;
    negative = TRUE;
    --keta;
  }
  char* p = uint2num(s, x, base, table, padding, keta);
  if (negative)
    *(--p) = '-';
  return p;
}

int vsprintf(char *str, const char *fmt, int* arg) {
  static const char hextableLower[] = "0123456789abcdef";
  static const char hextableUpper[] = "0123456789ABCDEF";
  char* dst = str;
  while (*fmt != '\0') {
    if (*fmt != '%') {
      *dst++ = *fmt++;
      continue;
    }

    int keta = 0;
    char padding = ' ';
    char buf[sizeof(int) * 3 + 3], *last = &buf[sizeof(buf)];
    char* q;
  again:
    switch (*(++fmt)) {
    default:
      *dst++ = *fmt++;
      continue;
    case '\0':
      continue;
    case '0': padding = '0'; goto again;
    case '1': case '2': case '3': case '4': case '5':
    case '6': case '7': case '8': case '9':
      keta = *fmt - '0';
      goto again;
    case 'd': q = int2num(last, *arg++, 10, hextableLower, padding, keta); break;
    case 'x': q = uint2num(last, *arg++, 16, hextableLower, padding, keta); break;
    case 'X': q = uint2num(last, *arg++, 16, hextableUpper, padding, keta); break;
    case 'p':
      q = uint2num(last, *arg++, 16, hextableLower, '0', sizeof(void*) * 2);
      *(--q) = 'x';
      *(--q) = '0';
      break;
    }
    strcpy(dst, q);
    dst += (last - q) - 1;
    ++fmt;
  }
  *dst = '\0';
  return dst - str;
}

int sprintf(char *str, const char *fmt, ...) {
  int* arg = (int*)(&(&fmt)[1]);  // Get va_arg
  return vsprintf(str, fmt, arg);
}
