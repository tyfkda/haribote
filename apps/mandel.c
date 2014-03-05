#include "apilib.h"

#define W  (256)
#define H  (256)
#define DEPTH  (256)

static int calcMandelbrotDepth(double xx, double yy, int depth) {
  double x = 0, y = 0;
  for (int c = 0; c < depth; ++c) {
    double nx = x * x - y * y + xx;
    double ny = 2 * x * y + yy;
    x = nx;
    y = ny;
    if (x * x + y * y >= 4)
      return c;
  }
  return -1;
}

static void drawMandelbrot(int win, int width, int height, int depth,
                           double x0, double y0, double x1, double y1) {
  for (int i = 0; i < height; ++i) {
    double yy = i * (y1 - y0) / height + y0;
    int y = i + 8 + 18;
    for (int j = 0; j < width; ++j) {
      double xx = j * (x1 - x0) / width + x0;
      int c = calcMandelbrotDepth(xx, yy, depth) + 1;
      api_point(win | 1, j + 8, y, (c % 216) + 16);
    }
    api_refreshwin(win, 8, y, 8 + width, y + 1);
  }
}

#define WIN_W  (W + 8 * 2)
#define WIN_H  (H + 8 * 2 + 18)
static unsigned char buf[WIN_W * WIN_H];

int main() {
  int win = api_openwin(buf, WIN_W, WIN_H, -1, "mandelbrot");
  api_boxfilwin(win, 8, 8 + 18, WIN_W - 8, WIN_H - 8, COL8_BLACK);

  drawMandelbrot(win, W, H, DEPTH, -2, -1.5, 1, 1.5);
  api_getkey(1);
  return 0;
}
