#include "apilib.h"

int main() {
  unsigned char* buf = api_malloc(144 * 164);
  int win = api_openwin(buf, 144, 164, -1, "color");
  for (int y = 0; y < 128; ++y) {
    for (int x = 0; x < 128; ++x) {
      int r = x * 2;
      int g = y * 2;
      int b = 0;
      buf[(x + 8) + (y + 28) * 144] = 16 + (r / 43) + (g / 43) * 6 + (b / 43) * 36;
    }
  }
  api_refreshwin(win, 8, 28, 136, 156);

  for (;;)
    if (api_getkey(1) == 0x1b)
      break;
  return 0;
}
