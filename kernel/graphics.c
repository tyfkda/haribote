#include "graphics.h"
#include "naskfunc.h"
#include "string.h"

extern const unsigned char fontdata[];

void init_palette(void) {
  static unsigned char table_rgb[16 * 3] = {
    0x00, 0x00, 0x00,  // 0: black
    0xff, 0x00, 0x00,  // 1: red
    0x00, 0xff, 0x00,  // 2: green
    0xff, 0xff, 0x00,  // 3: yellow
    0x00, 0x00, 0xff,  // 4: blue
    0xff, 0x00, 0xff,  // 5: purple
    0x00, 0xff, 0xff,  // 6: cyan
    0xff, 0xff, 0xff,  // 7: white
    0xc6, 0xc6, 0xc6,  // 8: gray
    0x84, 0x00, 0x00,  // 9: dark red
    0x00, 0x84, 0x00,  // 10: dark green
    0x84, 0x84, 0x00,  // 11: dark yellow
    0x00, 0x00, 0x84,  // 12: dark blue
    0x84, 0x00, 0x84,  // 13: dark purple
    0x00, 0x84, 0x84,  // 14: dark cyan
    0x84, 0x84, 0x84,  // 15: dark gray
  };
  set_palette(0, 15, table_rgb);

  unsigned char table2[216 * 3];
  for (int b = 0; b < 6; ++b) {
    for (int g = 0; g < 6; ++g) {
      for (int r = 0; r < 6; ++r) {
        unsigned char* p = &table2[(r + g * 6 + b * 36) * 3];
        p[0] = r * 51;
        p[1] = g * 51;
        p[2] = b * 51;
      }
    }
  }
  set_palette(16, 231, table2);
}

void set_palette(int start, int end, unsigned char* rgb) {
  int eflags = io_load_eflags();  // Save interrupt flag.
  io_cli();  // Prevent interrupt.
  io_out8(0x03c8, start);
  for (int i = start; i <= end; ++i) {
    io_out8(0x03c9, rgb[0] / 4);
    io_out8(0x03c9, rgb[1] / 4);
    io_out8(0x03c9, rgb[2] / 4);
    rgb += 3;
  }
  io_store_eflags(eflags);  // Restore interrupt flag.
}

void boxfill8(unsigned char* vram, int xsize, unsigned char c, int x0, int y0, int x1, int y1) {
  for (int y = y0; y < y1; ++y)
    for (int x = x0; x < x1; ++x)
      vram[y * xsize + x] = c;
}

void draw_shaded_box(unsigned char* buf, int xsize, int x0, int y0, int x1, int y1, unsigned char col_tl, unsigned char col_br, int col_center) {
  boxfill8(buf, xsize, col_tl, x0, y0, x1, y0 + 1);
  boxfill8(buf, xsize, col_tl, x0, y0 + 1, x0 + 1, y1);
  boxfill8(buf, xsize, col_br, x1 - 1, y0, x1, y1);
  boxfill8(buf, xsize, col_br, x0, y1 - 1, x1 - 1, y1);
  if (col_center >= 0)
    boxfill8(buf, xsize, col_center, x0 + 1, y0 + 1, x1 - 1, y1 - 1);
}

void line8(unsigned char* vram, int xsize,
           int x0, int y0, int x1, int y1, unsigned char c) {
  int dx = x1 - x0;
  int dy = y1 - y0;
  int x = x0 << 10;
  int y = y0 << 10;
  if (dx < 0)
    dx = -dx;
  if (dy < 0)
    dy = -dy;
  int len;
  if (dx >= dy) {
    len = dx + 1;
    dx = x0 > x1 ? -1024 : 1024;
    dy = y0 <= y1 ? ((y1 - y0 + 1) << 10) / len : ((y1 - y0 - 1) << 10) / len;
  } else {
    len = dy + 1;
    dy = y0 > y1 ? -1024 : 1024;
    dx = x0 <= x1 ? ((x1 - x0 + 1) << 10) / len : ((x1 - x0 - 1) << 10) / len;
  }
  for (int i = 0; i < len; ++i, x += dx, y += dy)
    vram[(y >> 10) * xsize + (x >> 10)] = c;
}

void init_screen8(unsigned char* vram, int x, int y) {
  boxfill8(vram, x, COL8_DARK_CYAN,  0,          0, x, y - 28);
  boxfill8(vram, x, COL8_GRAY,       0, y - 28, x, y - 27);
  boxfill8(vram, x, COL8_WHITE,      0, y - 27, x, y - 26);
  boxfill8(vram, x, COL8_GRAY,       0, y - 26, x, y);

  draw_shaded_box(vram, x, 2, y - 24, 61, y - 2, COL8_WHITE, COL8_BLACK, -1);
  draw_shaded_box(vram, x, 3, y - 23, 60, y - 3, COL8_GRAY, COL8_DARK_GRAY, -1);

  draw_shaded_box(vram, x, x - 47, y - 24, x - 2, y - 2, COL8_DARK_GRAY, COL8_WHITE, -1);
}

void putfont8(unsigned char* vram, int xsize, int ysize, int x, int y, unsigned char c, const unsigned char* font) {
  const int FONTW = 8, FONTH = 16;
  int x0 = 0, y0 = 0;
  int w = FONTW, h = FONTH;
  if (x < 0)
    x0 = -x;
  if (y < 0)
    y0 = -y;
  if (x + w > xsize)
    w = xsize - x;
  if (y + h > ysize)
    h = ysize - y;
  for (int i = y0; i < h; ++i)
    for (int j = x0; j < w; ++j)
      if (font[i] & (0x80 >> j))
	vram[(y + i) * xsize + x + j] = c;
}

void putfonts8_asc(unsigned char* vram, int xsize, int ysize, int x, int y, unsigned char c, const char* s) {
  const int FONTW = 8, FONTH = 16;
  if (y <= -FONTH || y >= ysize)
    return;
  if (x < 0) {
    unsigned int n = -x / FONTW;
    if (strlen(s) <= n)
      return;
    x += n * FONTW;
    s += n;
  }
  while (*s != '\0') {
    putfont8(vram, xsize, ysize, x, y, c, &fontdata[16 * (unsigned char)*s++]);
    x += 8;
    if (x >= xsize)
      break;
  }
}

void convert_image8(unsigned char* buf, int bufXsize, int x0, int y0, int imgXsize, int imgYsize, const char* image, const unsigned char* table) {
  for (int y = 0; y < imgYsize; ++y) {
    for (int x = 0; x < imgXsize; ++x) {
      unsigned char c = image[y * imgXsize + x];
      for (int i = 0; ; i += 2) {
        if (c == table[i]) {
          buf[(y + y0) * bufXsize + x + x0] = table[i + 1];
          break;
        }
      }
    }
  }
}

void init_mouse_cursor8(unsigned char* mouse, unsigned char bc) {
  static const char cursor[16][16] = {
    "**************..",
    "*ooooooooooo*...",
    "*oooooooooo*....",
    "*ooooooooo*.....",
    "*oooooooo*......",
    "*ooooooo*.......",
    "*ooooooo*.......",
    "*oooooooo*......",
    "*oooo**ooo*.....",
    "*ooo*..*ooo*....",
    "*oo*....*ooo*...",
    "*o*......*ooo*..",
    "**........*ooo*.",
    "*..........*ooo*",
    "............*oo*",
    ".............***",
  };
  unsigned char table[][2] = {
    { '*', COL8_BLACK },
    { 'o', COL8_WHITE },
    { '.', bc },
    { '\0' },
  };
  convert_image8(mouse, 16, 0, 0, 16, 16, &cursor[0][0], &table[0][0]);
}

void putblock8_8(unsigned char* vram, int xsize, int pxsize, int pysize,
                 int px0, int py0, const unsigned char* buf, int bxsize) {
  for (int y = 0; y < pysize; ++y) {
    for (int x = 0; x < pxsize; ++x) {
      unsigned char c = buf[y * bxsize + x];
      if (c != 0xff)
        vram[(py0 + y) * xsize + (px0 + x)] = c;
    }
  }
}
