#include "stdio.h"

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
