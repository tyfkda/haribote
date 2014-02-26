#include "apilib.h"
#include "stdlib.h"

unsigned char rgb2pal(int r, int g, int b, int x, int y) {
  static int table[4] = { 3, 1, 0, 2 };
  int i;
  x &= 1;
  y &= 1;
  i = table[x + y * 2];
  r = (((r * 21) >> 8) + i) >> 2;  // 0~5
  g = (((g * 21) >> 8) + i) >> 2;  // 0~5
  b = (((b * 21) >> 8) + i) >> 2;  // 0~5
  return 16 + r + g * 6 + b * 36;
}

int main() {
  unsigned char* buf = api_malloc(144 * 164);
  int win = api_openwin(buf, 144, 164, -1, "color2");
  for (int y = 0; y < 128; ++y)
    for (int x = 0; x < 128; ++x)
      buf[(x + 8) + (y + 28) * 144] = rgb2pal(x * 2, y * 2, 0, x, y);
  api_refreshwin(win, 8, 28, 136, 156);

  api_getkey(1);
  return 0;
}
