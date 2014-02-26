#include "stdio.h"
#include "string.h"
#include "apilib.h"

int puts(const char* str) {
  api_putstr0(str);
  putchar('\n');
  return strlen(str);
}
