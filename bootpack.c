void io_hlt(void);
void io_cli(void);
void io_out8(int port, int data);
int io_load_eflags(void);
void io_store_eflags(int eflags);

const int COL8_BLACK = 0;
const int COL8_WHITE = 7;
const int COL8_GRAY = 8;
const int COL8_DARK_CYAN = 14;
const int COL8_DARK_GRAY = 15;

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

void boxfill8(unsigned char* vram, int xsize, unsigned char c, int x0, int y0, int x1, int y1) {
  for (int y = y0; y < y1; ++y)
    for (int x = x0; x < x1; ++x)
      vram[y * xsize + x] = c;
}

void HariMain(void) {
  init_palette();

  unsigned char* vram = (unsigned char*)0xa0000;
  int xsize = 320;
  int ysize = 200;

  boxfill8(vram, xsize, COL8_DARK_CYAN,  0,          0, xsize, ysize - 28);
  boxfill8(vram, xsize, COL8_GRAY,       0, ysize - 28, xsize, ysize - 27);
  boxfill8(vram, xsize, COL8_WHITE,      0, ysize - 27, xsize, ysize - 26);
  boxfill8(vram, xsize, COL8_GRAY,       0, ysize - 26, xsize, ysize);

  boxfill8(vram, xsize, COL8_WHITE,      3, ysize - 24,    60, ysize - 23);
  boxfill8(vram, xsize, COL8_WHITE,      2, ysize - 24,     3, ysize - 3);
  boxfill8(vram, xsize, COL8_DARK_GRAY,  3, ysize -  4,    60, ysize - 3);
  boxfill8(vram, xsize, COL8_DARK_GRAY, 59, ysize - 23,    60, ysize - 4);
  boxfill8(vram, xsize, COL8_BLACK,      2, ysize -  3,    60, ysize - 2);
  boxfill8(vram, xsize, COL8_BLACK,     60, ysize - 24,    61, ysize - 2);

  boxfill8(vram, xsize, COL8_DARK_GRAY, xsize - 47, ysize - 24, xsize -  3, ysize - 23);
  boxfill8(vram, xsize, COL8_DARK_GRAY, xsize - 47, ysize - 23, xsize - 46, ysize - 3);
  boxfill8(vram, xsize, COL8_WHITE,     xsize - 47, ysize -  3, xsize -  3, ysize - 2);
  boxfill8(vram, xsize, COL8_WHITE,     xsize -  3, ysize - 24, xsize -  2, ysize - 2);

  for (;;)
    io_hlt();
}
