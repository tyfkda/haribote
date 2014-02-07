void io_hlt(void);
void io_cli(void);
void io_out8(int port, int data);
int io_load_eflags(void);
void io_store_eflags(int eflags);

void load_gdtr(int limit, int addr);
void load_idtr(int limit, int addr);

const int COL8_BLACK = 0;
const int COL8_WHITE = 7;
const int COL8_GRAY = 8;
const int COL8_DARK_CYAN = 14;
const int COL8_DARK_GRAY = 15;

extern const unsigned char fontdata[][16];

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

static char* int2num(char *s, int x, int base, const char* table) {
  *(--s) = '\0';
  do {
    *(--s) = table[x % base];
    x /= base;
  } while (x > 0);
  return s;
}

char* strcpy(char* dst, char* src) {
  char* orgDst = dst;
  for (;;) {
    if ((*dst++ = *src++) == '\0')
      break;
  }
  return orgDst;
}

int sprintf(char *str, const char *fmt, ...) {
  static const char hextable[] = "0123456789abcdef";
  int* arg = (int*)(&(&fmt)[1]);  // Get va_arg
  char* dst = str;
  while (*fmt != '\0') {
    if (*fmt != '%') {
      *dst++ = *fmt++;
      continue;
    }

    char buf[sizeof(int) * 3 + 1], *last = &buf[sizeof(buf)];
    char* q;
    switch (*(++fmt)) {
    default:
      *dst++ = *fmt++;
      continue;
    case 'd': q = int2num(last, *arg++, 10, hextable); break;
    case 'x': q = int2num(last, *arg++, 16, hextable); break;
    }
    strcpy(dst, q);
    dst += (last - q) - 1;
    ++fmt;
  }
  *dst = '\0';
  return dst - str;
}

void init_mouse_cursor8(unsigned char* mouse, char bc) {
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
      unsigned char c = bc;
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
  for (int y = 0; y < pysize; ++y)
    for (int x = 0; x < pxsize; ++x)
      vram[(py0 + y) * xsize + (px0 + x)] = buf[y * bxsize + x];
}

struct BOOTINFO {
  char cyls, leds, vmode, reserve;
  short scrnx, scrny;
  unsigned char* vram;
};

struct SEGMENT_DESCRIPTOR {
  short limit_low, base_low;
  char base_mid, access_right;
  char limit_high, base_high;
};

struct GATE_DESCRIPTOR {
  short offset_low, selector;
  char dw_count, access_right;
  short offset_high;
};

void set_segmdesc(struct SEGMENT_DESCRIPTOR* sd, unsigned int limit, int base, int ar) {
  if (limit > 0xfffff) {
    ar |= 0x8000;  // G_bit = 1
    limit /= 0x1000;
  }
  sd->limit_low = limit & 0xffff;
  sd->base_low = base & 0xffff;
  sd->base_mid = (base >> 16) & 0xff;
  sd->access_right = ar & 0xff;
  sd->limit_high = ((limit >> 16) & 0x0f) | ((ar >> 8) & 0xf0);
  sd->base_high = (base >> 24) & 0xff;
}

void set_gatedesc(struct GATE_DESCRIPTOR* gd, int offset, int selector, int ar) {
  gd->offset_low = offset & 0xffff;
  gd->selector = selector;
  gd->dw_count = (ar >> 8) & 0xff;
  gd->access_right = ar & 0xff;
  gd->offset_high = (offset >> 16) & 0xffff;
}

void init_gdtidt(void) {
  struct SEGMENT_DESCRIPTOR* gdt = (struct SEGMENT_DESCRIPTOR*)0x00270000;
  struct GATE_DESCRIPTOR* idt = (struct GATE_DESCRIPTOR*)0x0026f800;

  // Init GDT.
  for (int i = 0; i < 8192; ++i)
    set_segmdesc(gdt + i, 0, 0, 0);
  set_segmdesc(gdt + 1, 0xffffffff, 0x00000000, 0x4092);
  set_segmdesc(gdt + 2, 0x0007ffff, 0x00280000, 0x409a);
  load_gdtr(0xffff, 0x00270000);

  // Init IDT.
  for (int i = 0; i < 256; ++i)
    set_gatedesc(idt + i, 0, 0, 0);
  load_idtr(0x7ff, 0x0026f800);
}


void HariMain(void) {
  init_gdtidt();
  init_palette();

  struct BOOTINFO* binfo = (struct BOOTINFO*)0x0ff0;
  init_screen(binfo->vram, binfo->scrnx, binfo->scrny);

  unsigned char mcursor[256];
  init_mouse_cursor8(mcursor, COL8_DARK_CYAN);
  int mx = (binfo->scrnx - 16) / 2;
  int my = (binfo->scrny - 28 - 16) / 2;
  putblock8_8(binfo->vram, binfo->scrnx, 16, 16, mx, my, mcursor, 16);

  char s[40];
  sprintf(s, "(%d, %d)", mx, my);
  putfonts8_asc(binfo->vram, binfo->scrnx, 0, 0, COL8_WHITE, s);

  for (;;)
    io_hlt();
}
