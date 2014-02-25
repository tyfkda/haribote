#include "apilib.h"
//#include <math.h>

#define W  (256 + 16)
#define H  (256 + 16 + 18)
#define DEPTH  (256)

static void drawMandelbrot(int win) {
  for (int i = 0; i < 256; ++i) {
    double y0 = (i - 127.5) * (3.0 / 256);
    for (int j = 0; j < 256; ++j) {
      double x0 = j * (3.0 / 256) - 2.0;
      double x = 0, y = 0;
      for (int c = 0; c < DEPTH; ++c) {
        double nx = x * x - y * y + x0;
        double ny = 2 * x * y + y0;
        x = nx;
        y = ny;
        if (x * x + y * y >= 4) {
          api_point(win | 1, j + 8, i + 8 + 18, (c % 216) + 16);
          break;
        }
      }
      api_refreshwin(win, 8, i + 8 + 18, 8 + 256, i + 8 + 19);
    }
  }
}

static unsigned char buf[W * H];

void HariMain(void) {
  int win = api_openwin(buf, W, H, -1, "mandelbrot");
  api_boxfilwin(win, 8, 8 + 18, 8 + 256, 8 + 256 + 18, COL8_BLACK);

  drawMandelbrot(win);
  api_getkey(1);
  api_end();
}
