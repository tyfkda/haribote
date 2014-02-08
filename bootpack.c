#include "bootpack.h"
#include "dsctbl.h"
#include "fifo.h"
#include "graphic.h"
#include "int.h"
#include "keyboard.h"
#include "mouse.h"
#include "stdio.h"

#define EFLAGS_AC_BIT      0x00040000
#define CR0_CACHE_DISABLE  0x60000000

extern unsigned int memtest_sub(unsigned int start, unsigned int end) {
  const unsigned int pat0 = 0xaa55aa55, pat1 = ~pat0;
  unsigned int i;
  for (i = start; i <= end; i += 0x1000) {
    volatile unsigned int* p = (unsigned int*)(i + 0xffc);
    unsigned int old = *p;
    *p = pat0;
    *p ^= 0xffffffff;
    if (*p != pat1) {
    not_memory:
      *p = old;
      break;
    }
    *p ^= 0xffffffff;
    if (*p != pat0)
      goto not_memory;
    *p = old;
  }
  return i;
}

unsigned int memtest(unsigned int start, unsigned int end) {
  char flg486 = 0;

  unsigned int eflg = io_load_eflags();
  eflg |= EFLAGS_AC_BIT;  // AC-bit = 1
  io_store_eflags(eflg);
  eflg = io_load_eflags();
  if ((eflg & EFLAGS_AC_BIT) != 0)  // AC becomes 0 on 386.
    flg486 = 1;
  eflg &= ~EFLAGS_AC_BIT;  // AC-bit = 0
  io_store_eflags(eflg);

  if (flg486) {
    unsigned int cr0 = load_cr0();
    cr0 |= CR0_CACHE_DISABLE;  // Disable cache.
    store_cr0(cr0);
  }

  unsigned int i = memtest_sub(start, end);
  if (flg486) {
    unsigned int cr0 = load_cr0();
    cr0 &= ~CR0_CACHE_DISABLE;  // Enable cache.
    store_cr0(cr0);
  }
  return i;
}

void HariMain(void) {
  init_gdtidt();
  init_pic();
  io_sti();  // Enable CPU interrupt after IDT/PIC initialization.

  unsigned char keybuf[32], mousebuf[128];
  struct MOUSE_DEC mdec;
  fifo8_init(&keyfifo, 32, keybuf);
  fifo8_init(&mousefifo, 128, mousebuf);
  io_out8(PIC0_IMR, 0xf9);  // Enable PIC1 and keyboard.
  io_out8(PIC1_IMR, 0xef);  // Enable mouse.

  init_keyboard();
  enable_mouse(&mdec);

  struct BOOTINFO* binfo = (struct BOOTINFO*)ADR_BOOTINFO;
  init_palette();
  init_screen8(binfo->vram, binfo->scrnx, binfo->scrny);

  unsigned char mcursor[256];
  init_mouse_cursor8(mcursor);
  int mx = (binfo->scrnx - 16) / 2;
  int my = (binfo->scrny - 28 - 16) / 2;
  putblock8_8(binfo->vram, binfo->scrnx, 16, 16, mx, my, mcursor, 16);

  char s[40];
  sprintf(s, "(%3d, %3d)", mx, my);
  putfonts8_asc(binfo->vram, binfo->scrnx, 0, 0, COL8_WHITE, s);

  int i = memtest(0x00400000, 0xbfffffff) / (1024 * 1024);
  sprintf(s, "memory %dMB", i);
  putfonts8_asc(binfo->vram, binfo->scrnx, 0, 32, COL8_WHITE, s);

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
        sprintf(s, "(%3d, %3d)", mx, my);
        boxfill8(binfo->vram, binfo->scrnx, COL8_DARK_CYAN, 0, 0, 80, 16);
        putfonts8_asc(binfo->vram, binfo->scrnx, 0, 0, COL8_WHITE, s);
        putblock8_8(binfo->vram, binfo->scrnx, 16, 16, mx, my, mcursor, 16);
      }
      continue;
    }

    io_stihlt();
  }
}
