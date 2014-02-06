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

void init_screen(unsigned char* vram, int x, int y) {
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

struct BOOTINFO {
  char cyls, leds, vmode, reserve;
  short scrnx, scrny;
  unsigned char* vram;
};

void HariMain(void) {
  init_palette();

  struct BOOTINFO* binfo = (struct BOOTINFO*)0x0ff0;
  init_screen(binfo->vram, binfo->scrnx, binfo->scrny);

  for (;;)
    io_hlt();
}
