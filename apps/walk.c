#include "apilib.h"

int main() {
  unsigned char* buf = api_malloc(160 * 100);
  int win = api_openwin(buf, 160, 100, -1, "walk");
  api_boxfilwin(win, 4, 24, 155, 95, 0);
  int x = 76, y = 56;
  api_putstrwin(win, x, y, 3, 1, "*");
  for (;;) {
    int i = api_getkey(1);
    api_putstrwin(win, x, y, 0, 1, "*");  // Erase
    if (i == KEY_LEFT && x > 4)  x -= 8;
    if (i == KEY_RIGHT && x < 148)  x += 8;
    if (i == KEY_UP && y > 24)  y -= 8;
    if (i == KEY_DOWN && y < 80)  y += 8;
    if (i == 0x0a)  break;
    api_putstrwin(win, x, y, 3, 1, "*");  // Erase
  }
  api_closewin(win);
  return 0;
}
