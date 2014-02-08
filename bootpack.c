#include "bootpack.h"
#include "dsctbl.h"
#include "fifo.h"
#include "graphic.h"
#include "int.h"
#include "stdio.h"

static const int PORT_KEYDAT = 0x0060;
static const int PORT_KEYSTA = 0x0064;
static const int PORT_KEYCMD = 0x0064;
static const int KEYSTA_SEND_NOTREADY = 0x02;
static const int KEYCMD_WRITE_MODE = 0x60;
static const int KBC_MODE = 0x47;

static const int KEYCMD_SENDTO_MOUSE = 0xd4;
static const int MOUSECMD_ENABLE = 0xf4;

void wait_KBC_sendready(void) {
  for (;;)
    if ((io_in8(PORT_KEYSTA) & KEYSTA_SEND_NOTREADY) == 0)
      break;
}

void init_keyboard(void) {
  wait_KBC_sendready();
  io_out8(PORT_KEYCMD, KEYCMD_WRITE_MODE);
  wait_KBC_sendready();
  io_out8(PORT_KEYDAT, KBC_MODE);
}

void enable_mouse(void) {
  wait_KBC_sendready();
  io_out8(PORT_KEYCMD, KEYCMD_SENDTO_MOUSE);
  wait_KBC_sendready();
  io_out8(PORT_KEYDAT, MOUSECMD_ENABLE);
}

void HariMain(void) {
  init_gdtidt();
  init_pic();
  io_sti();  // Enable CPU interrupt after IDT/PIC initialization.

  unsigned char keybuf[32], mousebuf[128];
  fifo8_init(&keyfifo, 32, keybuf);
  fifo8_init(&mousefifo, 128, mousebuf);
  io_out8(PIC0_IMR, 0xf9);  // Enable PIC1 and keyboard.
  io_out8(PIC1_IMR, 0xef);  // Enable mouse.

  init_keyboard();

  struct BOOTINFO* binfo = (struct BOOTINFO*)ADR_BOOTINFO;
  init_palette();
  init_screen8(binfo->vram, binfo->scrnx, binfo->scrny);

  unsigned char mcursor[256];
  init_mouse_cursor8(mcursor, COL8_DARK_CYAN);
  int mx = (binfo->scrnx - 16) / 2;
  int my = (binfo->scrny - 28 - 16) / 2;
  putblock8_8(binfo->vram, binfo->scrnx, 16, 16, mx, my, mcursor, 16);

  char s[40];
  sprintf(s, "(%d, %d)", mx, my);
  putfonts8_asc(binfo->vram, binfo->scrnx, 0, 0, COL8_WHITE, s);

  enable_mouse();

  for (;;) {
    io_cli();
    if (fifo8_status(&keyfifo) != 0) {
      int i = fifo8_get(&keyfifo);
      io_sti();

      char s[4];
      sprintf(s, "%02x", i);
      boxfill8(binfo->vram, binfo->scrnx, COL8_DARK_CYAN, 0, 16, 16, 32);
      putfonts8_asc(binfo->vram, binfo->scrnx, 0, 16, COL8_WHITE, s);
      continue;
    }
    if (fifo8_status(&mousefifo) != 0) {
      int i = fifo8_get(&mousefifo);
      io_sti();

      char s[4];
      sprintf(s, "%02x", i);
      boxfill8(binfo->vram, binfo->scrnx, COL8_DARK_CYAN, 32, 16, 48, 32);
      putfonts8_asc(binfo->vram, binfo->scrnx, 32, 16, COL8_WHITE, s);
      continue;
    }

    io_stihlt();
  }
}
