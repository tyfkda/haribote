#include "apilib.h"

int main() {
  unsigned char* buf = api_malloc(160 * 100);
  int win = api_openwin(buf, 160, 100, -1, "lines");
  for (int i = 0; i < 8; ++i) {
    api_linewin(win | 1,  8, 26, 77, i * 9 + 26, i);
    api_linewin(win | 1, 88, 26, i * 9 + 88, 89, i);
  }
  api_refreshwin(win, 6, 26, 154, 90);

  for (;;)
    if (api_getkey(1) == 0x0a)  // Enter
      break;
  api_closewin(win);
  return 0;
}
