#include "graphic.h"
#include "bootpack.h"

extern const unsigned char fontdata[][16];

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

void init_screen8(unsigned char* vram, int x, int y) {
  boxfill8(vram, x, COL8_DARK_CYAN,  0,          0, x, y - 28);
  boxfill8(vram, x, COL8_GRAY,       0, y - 28, x, y - 27);
  boxfill8(vram, x, COL8_WHITE,      0, y - 27, x, y - 26);
  boxfill8(vram, x, COL8_GRAY,       0, y - 26, x, y);

  boxfill8(vram, x, COL8_WHITE,      3, y - 24,    60, y - 23);
  boxfill8(vram, x, COL8_WHITE,      2, y - 24,     3, y - 3);
  boxfill8(vram, x, COL8_DARK_GRAY,  3, y -  4,    60, y - 3);
  boxfill8(vram, x, COL8_DARK_GRAY, 59, y - 23,    60, y - 4);
  boxfill8(vram, x, COL8_BLACK,      2, y -  3,    60, y - 2);
  boxfill8(vram, x, COL8_BLACK,     60, y - 24,    61, y - 2);

  boxfill8(vram, x, COL8_DARK_GRAY, x - 47, y - 24, x -  3, y - 23);
  boxfill8(vram, x, COL8_DARK_GRAY, x - 47, y - 23, x - 46, y - 3);
  boxfill8(vram, x, COL8_WHITE,     x - 47, y -  3, x -  3, y - 2);
  boxfill8(vram, x, COL8_WHITE,     x -  3, y - 24, x -  2, y - 2);
}

void putfont8(unsigned char* vram, int xsize, int x, int y, unsigned char c, const unsigned char* font) {
  for (int i = 0; i < 16; ++i)
    for (int j = 0; j < 8; ++j)
      if (font[i] & (0x80 >> j))
	vram[(y + i) * xsize + x + j] = c;
}

void putfonts8_asc(unsigned char* vram, int xsize, int x, int y, unsigned char c, const char* s) {
  while (*s != '\0') {
    putfont8(vram, xsize, x, y, c, fontdata[(unsigned char)*s++]);
    x += 8;
  }
}

void init_mouse_cursor8(unsigned char* mouse) {
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
  for (int y = 0; y < 16; ++y) {
    for (int x = 0; x < 16; ++x) {
      unsigned char c = 0xff;
      switch (cursor[y][x]) {
      case '*':  c = COL8_BLACK; break;
      case 'o':  c = COL8_WHITE; break;
      }
      mouse[y * 16 + x] = c;
    }
  }
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
