//#include "ctype.h"
#include "stdlib.h"

int toupper(int c) {
  if (c < 'a' || c > 'z')
    return c;
  return c - ('A' - 'a');
}
