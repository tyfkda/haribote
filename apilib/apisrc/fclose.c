#include "stdio.h"
#include "stdio_def.h"
#include "apilib.h"
#include "stdlib.h"

void fclose(FILE* fp) {
  if (fp != NULL) {
    api_fclose(fp->handle);
    fp->handle = 0;
    free(fp);
  }
}
