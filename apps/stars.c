#include "api.h"

void HariMain(void) {
  api_initmalloc();
  unsigned char* buf = api_malloc(150 * 100);
  int win = api_openwin(buf, 150, 100, -1, "stars");
  api_boxfilwin(win, 6, 26, 143, 93, 0);
  for (int i = 0; i < 50; ++i) {
    int x = rand() % 137 + 6;
    int y = rand() % 67 + 26;
    api_point(win | 1, x, y, 3);
  }
  api_refresh(win, 6, 26, 144, 94);

  for (;;)
    if (api_getkey(1) == 0x0a)  // Enter
      break;
  api_closewin(win);
  api_end();
}
