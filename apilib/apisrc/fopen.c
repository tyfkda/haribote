#include "stdio.h"
#include "stdio_def.h"
#include "apilib.h"
#include "stdlib.h"

FILE* fopen(const char* filename, const char* mode) {
  FILE* fp = malloc(sizeof(FILE));
  if (fp != NULL) {
    int flag = 0;
    if (*mode == 'w')
      flag |= OPEN_WRITE;
    fp->handle = api_fopen(filename, flag);
  }
  return fp;
}
