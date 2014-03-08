#include "stdio.h"
#include "string.h"
#include "apilib.h"

int putc(int c, FILE *stream) {
  // TODO: Use stream.
  (void)stream;
  return putchar(c);
}
