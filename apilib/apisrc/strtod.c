#include "stdlib.h"
#include "math.h"
#include "stddef.h"

double strtod(const char *nptr, char **endptr) {
  char* tmp;
  if (endptr == NULL)
    endptr = &tmp;

  long deci = strtol(nptr, endptr, 10);
  if (**endptr != '.')
    return (double)deci;

  const char* num = *endptr + 1;
  long val = strtol(num, endptr, 10);
  return (double)deci + (double)val / pow(10, *endptr - num);
}
