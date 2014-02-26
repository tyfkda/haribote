#include "apilib.h"
#include "stdio.h"
#include "stdlib.h"

extern int main(int argc, char* argv[]);

static char* skipSpace(char* p) {
  for (; *p == ' '; ++p);
  return p;
}

static char* skipUntilSpace(char* p) {
  for (; *p != ' ' && *p != '\0'; ++p);
  return p;
}

void HariMain(void) {
  api_initmalloc();

  char cmdline[128];
  api_cmdline(cmdline, 128);  // TODO: Check command line argument length.

  int argc = 0;
  char* argv[16];

  char* p = cmdline;
  // TODO: Check argument count not to overflow.
  // TODO: Handle quotation.
  for (;;) {
    p = skipSpace(p);
    if (*p == '\0')
      break;
    argv[argc++] = p;
    p = skipUntilSpace(p);
    if (*p == '\0')
      break;
    *p++ = '\0';
  }

  int result = main(argc, argv);
  exit(result);
}
