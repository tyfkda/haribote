#include "stdio.h"

int strlen(const char* str) {
  int len = 0;
  for (; *str != '\0'; ++str, ++len);
  return len;
}
