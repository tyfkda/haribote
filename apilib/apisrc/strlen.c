#include "stdio.h"

size_t strlen(const char* str) {
  size_t len = 0;
  for (; *str != '\0'; ++str, ++len);
  return len;
}
