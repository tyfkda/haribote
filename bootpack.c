#include "bootpack.h"
#include "dsctbl.h"
#include "fifo.h"
#include "graphics.h"
#include "int.h"
#include "keyboard.h"
#include "memory.h"
#include "mouse.h"
#include "sheet.h"
#include "stdio.h"

void HariMain(void) {
  init_gdtidt();
  init_pic();
  io_sti();  // Enable CPU interrupt after IDT/PIC initialization.

  unsigned char keybuf[32], mousebuf[128];
  MOUSE_DEC mdec;
  fifo8_init(&keyfifo, 32, keybuf);
  fifo8_init(&mousefifo, 128, mousebuf);
  io_out8(PIC0_IMR, 0xf9);  // Enable PIC1 and keyboard.
  io_out8(PIC1_IMR, 0xef);  // Enable mouse.

  init_keyboard();
  enable_mouse(&mdec);

  unsigned int memtotal = memtest(0x00400000, 0xbfffffff);
  MEMMAN *memman = (MEMMAN*)MEMMAN_ADDR;
  memman_init(memman);
  memman_free(memman, 0x00001000, 0x0009e000); /* 0x00001000 - 0x0009efff */
  memman_free(memman, 0x00400000, memtotal - 0x00400000);

  BOOTINFO* binfo = (BOOTINFO*)ADR_BOOTINFO;
  init_palette();
  SHTCTL* shtctl = shtctl_init(memman, binfo->vram, binfo->scrnx, binfo->scrny);
  SHEET* sht_back = sheet_alloc(shtctl);
  SHEET* sht_mouse = sheet_alloc(shtctl);
  unsigned char* buf_back = (unsigned char*)memman_alloc_4k(memman, binfo->scrnx * binfo->scrny);
  sheet_setbuf(sht_back, buf_back, binfo->scrnx, binfo->scrny, -1);
  unsigned char buf_mouse[16 * 16];
  sheet_setbuf(sht_mouse, buf_mouse, 16, 16, 99);
  init_screen8(buf_back, binfo->scrnx, binfo->scrny);

  init_mouse_cursor8(buf_mouse, 99);
  int mx = (binfo->scrnx - 16) / 2;
  int my = (binfo->scrny - 28 - 16) / 2;
  sheet_slide(shtctl, sht_back, 0, 0);
  sheet_slide(shtctl, sht_mouse, mx, my);
  sheet_updown(shtctl, sht_back, 0);
  sheet_updown(shtctl, sht_mouse, 1);

  char s[40];
  sprintf(s, "(%3d, %3d)", mx, my);
  putfonts8_asc(buf_back, binfo->scrnx, 0, 0, COL8_WHITE, s);

  sprintf(s, "memory %dMB   free : %dKB",
          memtotal / (1024 * 1024), memman_total(memman) / 1024);
  putfonts8_asc(buf_back, binfo->scrnx, 0, 32, COL8_WHITE, s);

  sheet_refresh(shtctl, sht_back, 0, 0, binfo->scrnx, 48);

  for (;;) {
    io_cli();
    if (fifo8_status(&keyfifo) != 0) {
      int i = fifo8_get(&keyfifo);
      io_sti();

      char s[4];
      sprintf(s, "%02X", i);
      boxfill8(buf_back, binfo->scrnx, COL8_DARK_CYAN, 0, 16, 16, 32);
      putfonts8_asc(buf_back, binfo->scrnx, 0, 16, COL8_WHITE, s);
      sheet_refresh(shtctl, sht_back, 0, 16, 16, 32);
      continue;
    }
    if (fifo8_status(&mousefifo) != 0) {
      int i = fifo8_get(&mousefifo);
      io_sti();
      if (mouse_decode(&mdec, i) != 0) {
        // Erase mouse cursor.
        sprintf(s, "[lcr %4d %4d]", mdec.x, mdec.y);
        if ((mdec.btn & 0x01) != 0)
          s[1] = 'L';
        if ((mdec.btn & 0x02) != 0)
          s[1] = 'R';
        if ((mdec.btn & 0x04) != 0)
          s[1] = 'C';
        boxfill8(buf_back, binfo->scrnx, COL8_DARK_CYAN, 32, 16, 32 + 15 * 8, 32);
        putfonts8_asc(buf_back, binfo->scrnx, 32, 16, COL8_WHITE, s);
        sheet_refresh(shtctl, sht_back, 32, 16, 32 + 15 * 8, 32);

        // Move mouse cursor.
        mx += mdec.x;
        my += mdec.y;
        if (mx < 0)  mx = 0;
        if (my < 0)  my = 0;
        if (mx >= binfo->scrnx - 1)  mx = binfo->scrnx - 1;
        if (my >= binfo->scrny - 1)  my = binfo->scrny - 1;
        sprintf(s, "(%3d, %3d)", mx, my);
        boxfill8(buf_back, binfo->scrnx, COL8_DARK_CYAN, 0, 0, 80, 16);
        putfonts8_asc(buf_back, binfo->scrnx, 0, 0, COL8_WHITE, s);
        sheet_refresh(shtctl, sht_back, 0, 0, 80, 16);
        sheet_slide(shtctl, sht_mouse, mx, my);
      }
      continue;
    }

    io_stihlt();
  }
}
