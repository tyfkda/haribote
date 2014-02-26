#include "apilib.h"
#include "stdio.h"
#include "stdlib.h"

void HariMain(void) {
  api_initmalloc();
  unsigned char* buf = api_malloc(150 * 50);
  int win = api_openwin(buf, 150, 50, -1, "noodle");
  TIMER* timer = api_alloctimer();
  api_inittimer(timer, 128);
  int sec = 0, min = 0, hou = 0;
  for (;;) {
    char s[12];
    sprintf(s, "%5d:%02d:%02d", hou, min, sec);
    api_boxfilwin(win, 28, 27, 115, 41, COL8_WHITE);
    api_putstrwin(win, 28, 27, 0, 11, s);
    api_settimer(timer, 100);
    if (api_getkey(1) != 128)
      break;
    if (++sec >= 60) {
      sec = 0;
      if (++min >= 60) {
        min = 0;
        ++hou;
      }
    }
  }
  exit(0);
}
