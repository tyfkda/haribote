#include "stdlib.h"

unsigned long int strtoul(const char *nptr, char **endptr, int base) {
  long v = strtol(nptr, endptr, base);
  return (unsigned long)v;
}
