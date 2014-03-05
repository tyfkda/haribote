#include "apilib.h"
#include "stdlib.h"
#include "stdio.h"

#define W  (128)
#define H  (128)
#define WIN_W  (W + 8 * 2)
#define WIN_H  (H + 8 * 2 + 18)
#define BLUR  (5)

void initField(unsigned char* cell, int w, int h) {
  for (int i = 0; i < w * h; ++i)
    cell[i] = (rand() & 0x4000) != 0 ? 1 : 0;
}

void stepLife(unsigned char* dst, const unsigned char* src, int w, int h) {
  static const unsigned char tbl[][9] = {
    { 0, 0, 0, 1, 0, 0, 0, 0, 0 },
    { 0, 0, 1, 1, 0, 0, 0, 0, 0 },
  };
  for (int y = 0; y < h; ++y) {
    int ys[] = { (y + h - 1) % h, y, (y + 1) % h };
    for (int x = 0; x < w; ++x) {
      int xs[] = { (x + w - 1) % w, x, (x + 1) % w };
      unsigned char cur = src[y * w + x];
      int n = cur == 1 ? -1 : 0;
      for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j)
          n += src[ys[i] * w + xs[j]] == 1 ? 1 : 0;

      if (cur == 0)
        dst[y * w + x] = tbl[0][n];
      else if (cur != 1)
        dst[y * w + x] = tbl[0][n] ? 1 : (cur < BLUR + 2) ? cur + 1 : 0;
      else
        dst[y * w + x] = tbl[1][n] ? 1 : 2;
    }
  }
}

void drawFiled(unsigned char* buf, int bx, const unsigned char* cell, int w, int h) {
  static const unsigned char colors[] = {
    COL8_BLACK, COL8_WHITE,
    5 * (6 * 6) + 16,
    4 * (6 * 6) + 16,
    3 * (6 * 6) + 16,
    2 * (6 * 6) + 16,
    1 * (6 * 6) + 16,
  };
  for (int y = 0; y < h; ++y) {
    unsigned char* p = &buf[(y + 8 + 18) * bx + 8];
    for (int x = 0; x < w; ++x)
      *p++ = colors[cell[y * w + x]];
  }
}

int main() {
  unsigned char* buf = malloc(WIN_W * WIN_H);
  int win = api_openwin(buf, WIN_W, WIN_H, -1, "Life game");
  api_boxfilwin(win, 8, 8 + 18, WIN_W - 8, WIN_H - 8, COL8_BLACK);

  unsigned char* cellBuffer = malloc(W * H * 2);
  unsigned char* cell = &cellBuffer[0];
  unsigned char* cell2 = &cellBuffer[W * H];

  TIMER* timer = api_alloctimer();
  api_inittimer(timer, 128);

 reset:
  initField(cell, W, H);
  for (;;) {
    drawFiled(buf, WIN_W, cell, W, H);
    api_refreshwin(win, 8, 8 + 18, WIN_W - 8, WIN_H - 8);

    api_settimer(timer, 10);
    for (;;) {
      int k = api_getkey(1);
      if (k == 128)
        break;
      if (k == 0x1b)
        goto exit;
      if (k == ' ')
        goto reset;
    }

    stepLife(cell2, cell, W, H);
    { unsigned char* tmp = cell; cell = cell2; cell2 = tmp; }
  }
 exit:
  return 0;
}
