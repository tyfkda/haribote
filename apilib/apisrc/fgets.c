#include "stdio.h"
#include "stdio_def.h"

char *fgets(char *s, int size, FILE *stream) {
  char* p = s;
  for (; size > 0; --size) {
    int c = fgetc(stream);
    if (c == EOF)
      break;
    *p++ = c;
    if (c == '\n')
      break;
  }
  if (p == s)  // EOF
    return NULL;
  // TODO: Confirm not to over buffer size.
  *p = '\0';
  return s;
}
