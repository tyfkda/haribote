#include "stdio.h"

static char* int2num(char *s, int x, int base, const char* table,
                     char padding, int keta) {
  int negative = FALSE;
  if (x < 0) {
    x = -x;
    negative = TRUE;
  }
  *(--s) = '\0';
  do {
    *(--s) = table[x % base];
    x /= base;
    --keta;
  } while (x > 0);
  if (negative) {
    *(--s) = '-';
    --keta;
  }
  for (; keta > 0; --keta)
    *(--s) = padding;
  return s;
}

int toupper(int c) {
  return ('a' <= c && c <= 'z') ? c - ('a' - 'A') : c;
}

void* memset(void* buf, int ch, int n) {
  char* p = (char*)buf;
  for (int i = 0; i < n; ++i)
    *p++ = ch;
  return buf;
}

void* memcpy(void* dst, const void* src, int size) {
  char* p = (char*)dst;
  const char* q = (const char*)src;
  for (; size > 0; --size)
    *p++ = *q++;
  return dst;
}

int strlen(const char* str) {
  int len = 0;
  for (; *str != '\0'; ++str, ++len);
  return len;
}

char* strcpy(char* dst, const char* src) {
  char* orgDst = dst;
  for (;;) {
    if ((*dst++ = *src++) == '\0')
      break;
  }
  return orgDst;
}

char* strncpy(char* dst, const char* src, int n) {
  char* orgDst = dst;
  for (; n > 0; --n) {
    if ((*dst++ = *src++) == '\0')
      break;
  }
  return orgDst;
}

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

int strncmp(const char* s1, const char* s2, int n) {
  for (unsigned char *u1 = (unsigned char*)s1, *u2 = (unsigned char*)s2;
       n > 0; ++u1, ++u2, --n) {
    int d = *u1 - *u2;
    if (d != 0)
      return d;
    if (*u1 == '\0')
      return 0;
  }
  return 0;
}

int sprintf(char *str, const char *fmt, ...) {
  static const char hextableLower[] = "0123456789abcdef";
  static const char hextableUpper[] = "0123456789ABCDEF";
  int* arg = (int*)(&(&fmt)[1]);  // Get va_arg
  char* dst = str;
  while (*fmt != '\0') {
    if (*fmt != '%') {
      *dst++ = *fmt++;
      continue;
    }

    int keta = 0;
    char padding = ' ';
    char buf[sizeof(int) * 3 + 1], *last = &buf[sizeof(buf)];
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
    case 'x': q = int2num(last, *arg++, 16, hextableLower, padding, keta); break;
    case 'X': q = int2num(last, *arg++, 16, hextableUpper, padding, keta); break;
    }
    strcpy(dst, q);
    dst += (last - q) - 1;
    ++fmt;
  }
  *dst = '\0';
  return dst - str;
}
