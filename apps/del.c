#include "apilib.h"
#include "stdio.h"

int main() {
  char cmdline[30];
  api_cmdline(cmdline, sizeof(cmdline));
  char* p;
  for (p = cmdline; *p > ' '; ++p);
  for (; *p == ' '; ++p);

  if (!api_delete(p)) {
    printf("File not found: %s\n", p);
    return 1;
  }
  return 0;
}
