#include "apilib.h"

int main() {
  unsigned char buf[150 * 50];
  int win = api_openwin(buf, 150, 50, -1, "hello");
  api_boxfilwin(win, 8, 36, 141, 43, 3);
  api_putstrwin(win, 28, 28, COL8_BLACK, 12, "hello, world");

  for (;;)
    if (api_getkey(1) == 0x1b)  // Escape
      break;
  api_closewin(win);
  return 0;
}
