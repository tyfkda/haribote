#include "api.h"

void HariMain() {
  char cmdline[30];
  api_cmdline(cmdline, sizeof(cmdline));
  char* p;
  for (p = cmdline; *p > ' '; ++p);
  for (; *p == ' '; ++p);

  const char* filename = p;
  int fh = api_fopen(filename);
  if (fh != 0) {
    int size = api_fsize(fh, 0);
    for (;;) {
      char c;
      if (api_fread(&c, 1, fh) == 0)
        break;
      api_putchar(c);
    }
    char s[30];
    sprintf(s, "\n#Size = %d\n", size);
    api_putstr0(s);
  } else {
    char s[50];
    sprintf(s, "File not found: %s\n", filename);
    api_putstr0(s);
  }
  api_end();
}
