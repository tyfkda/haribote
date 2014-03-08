#include "stdlib.h"
#include "stddef.h"

int atoi(const char *nptr) {
  long v = strtol(nptr, NULL, 10);
  return (int)v;
}
