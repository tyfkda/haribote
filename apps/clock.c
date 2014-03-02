#include "apilib.h"
#include "stdio.h"
#include "math.h"

static void draw_poly_circle(int win, int cx, int cy, int r, int div, int col) {
  int x = cx + r, y = cy;
  for (int i = 1; i <= div; ++i) {
    double t = i * (2 * M_PI) / div;
    int nx = cx + r * cos(t);
    int ny = cy + r * sin(t);
    api_linewin(win, x, y, nx, ny, col);
    x = nx;
    y = ny;
  }
}

static void draw_clock_line(int win, int val, int total, int r, int col) {
  const int X0 = 160 / 2 + 8, Y0 = 160 / 2 + 8 + 20;
  double t = val * (2 * M_PI) / total;
  int x = X0 + r * sin(t);
  int y = Y0 - r * cos(t);
  api_linewin(win, X0, Y0, x, y, col);
}

int main() {
  const int W = 160 + 8 * 2, H = 160 + 20 + 16 + 8 * 2;
  unsigned char* buf = api_malloc(W * H);
  int win = api_openwin(buf, W, H, -1, "clock");
  TIMER* timer = api_alloctimer();
  api_inittimer(timer, 128);
  for (;;) {
    unsigned char now[7];
    api_now(now);
    int year = (now[0] << 8) | now[1];
    int month = now[2];
    int day = now[3];
    int hour = now[4] % 12;
    int minute = now[5];
    int second = now[6];

    char s[24];
    sprintf(s, "%d/%02d/%02d %02d:%02d:%02d", year, month, day, hour, minute, second);
    api_boxfilwin(win | 1, 8, 20 + 8, W - 8, H - 8, COL8_DARK_GRAY);
    draw_poly_circle(win | 1, 160 / 2 + 8, 160 / 2 + 8 + 20, 78, 12, COL8_BLACK);
    draw_clock_line(win | 1, second, 60, 70, COL8_DARK_BLUE);
    draw_clock_line(win | 1, minute * 60 + second, 60 * 60, 60, COL8_GREEN);
    draw_clock_line(win | 1, (hour * 60 + minute) * 60 + second, 12 * 60 * 60, 40, COL8_RED);
    api_putstrwin(win | 1, 8, H - 8 - 16, 0, 19, s);
    api_refreshwin(win, 8, 20 + 8, W - 8, H - 8);

    api_settimer(timer, 100);
    if (api_getkey(1) != 128)
      break;
  }
  return 0;
}
