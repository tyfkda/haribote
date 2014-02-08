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

struct MOUSE_DEC {
  unsigned char buf[3], phase;
  int x, y, btn;
};

void enable_mouse(struct MOUSE_DEC* mdec) {
  wait_KBC_sendready();
  io_out8(PORT_KEYCMD, KEYCMD_SENDTO_MOUSE);
  wait_KBC_sendready();
  io_out8(PORT_KEYDAT, MOUSECMD_ENABLE);
  // ACK(0xfa) will be sent.
  mdec->phase = 0;
}

int mouse_decode(struct MOUSE_DEC* mdec, unsigned dat) {
  switch (mdec->phase) {
  case 0:  // Waiting 0xfa.
    if (dat == 0xfa)
      mdec->phase = 1;
    return 0;
  case 1:  // Waiting first byte.
    if ((dat & 0xc8) == 0x08) {
      mdec->buf[0] = dat;
      mdec->phase = 2;
    }
    return 0;
  case 2:  // Waiting second byte.
    mdec->buf[1] = dat;
    mdec->phase = 3;
    return 0;
  case 3:  // Waiting thrid byte.
    mdec->buf[2] = dat;
    mdec->phase = 1;
    mdec->btn = mdec->buf[0] & 0x07;
    mdec->x = mdec->buf[1];
    mdec->y = mdec->buf[2];
    if ((mdec->buf[0] & 0x10) != 0)
      mdec->x |= -1 << 8;
    if ((mdec->buf[0] & 0x20) != 0)
      mdec->y |= -1 << 8;
    mdec->y = -mdec->y;
    return 1;
  }
  return -1;
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
  init_mouse_cursor8(mcursor);
  int mx = (binfo->scrnx - 16) / 2;
  int my = (binfo->scrny - 28 - 16) / 2;
  putblock8_8(binfo->vram, binfo->scrnx, 16, 16, mx, my, mcursor, 16);

  char s[40];
  sprintf(s, "(%d, %d)", mx, my);
  putfonts8_asc(binfo->vram, binfo->scrnx, 0, 0, COL8_WHITE, s);

  struct MOUSE_DEC mdec;
  enable_mouse(&mdec);

  for (;;) {
    io_cli();
    if (fifo8_status(&keyfifo) != 0) {
      int i = fifo8_get(&keyfifo);
      io_sti();

      char s[4];
      sprintf(s, "%02X", i);
      boxfill8(binfo->vram, binfo->scrnx, COL8_DARK_CYAN, 0, 16, 16, 32);
      putfonts8_asc(binfo->vram, binfo->scrnx, 0, 16, COL8_WHITE, s);
      continue;
    }
    if (fifo8_status(&mousefifo) != 0) {
      int i = fifo8_get(&mousefifo);
      io_sti();
      if (mouse_decode(&mdec, i) != 0) {
        // Erase mouse cursor.
        boxfill8(binfo->vram, binfo->scrnx, COL8_DARK_CYAN, mx, my, mx + 16, my + 16);

        sprintf(s, "[lcr %4d %4d]", mdec.x, mdec.y);
        if ((mdec.btn & 0x01) != 0)
          s[1] = 'L';
        if ((mdec.btn & 0x02) != 0)
          s[1] = 'R';
        if ((mdec.btn & 0x04) != 0)
          s[1] = 'C';
        boxfill8(binfo->vram, binfo->scrnx, COL8_DARK_CYAN, 32, 16, 32 + 15 * 8, 32);
        putfonts8_asc(binfo->vram, binfo->scrnx, 32, 16, COL8_WHITE, s);

        // Move mouse cursor.
        mx += mdec.x;
        my += mdec.y;
        if (mx < 0)  mx = 0;
        if (my < 0)  my = 0;
        if (mx >= binfo->scrnx - 16)  mx = binfo->scrnx - 16;
        if (my >= binfo->scrny - 16)  my = binfo->scrny - 16;
        sprintf(s, "(%d, %d)", mx, my);
        boxfill8(binfo->vram, binfo->scrnx, COL8_DARK_CYAN, 0, 0, 80, 16);
        putfonts8_asc(binfo->vram, binfo->scrnx, 0, 0, COL8_WHITE, s);
        putblock8_8(binfo->vram, binfo->scrnx, 16, 16, mx, my, mcursor, 16);
      }
      continue;
    }

    io_stihlt();
  }
}
