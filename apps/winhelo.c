#include "apilib.h"
#include "stdio.h"

void HariMain(void) {
  api_initmalloc();
  unsigned char buf[150 * 50];
  int win = api_openwin(buf, 150, 50, -1, "hello");
  api_boxfilwin(win, 8, 36, 141, 43, 3);
  api_putstrwin(win, 28, 28, 0, 12, "hello, world");

  for (;;)
    if (api_getkey(1) == 0x0a)  // Enter
      break;
  api_closewin(win);
  exit(0);
}
