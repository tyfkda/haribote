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
#include "timer.h"

void make_window8(unsigned char* buf, int xsize, int ysize, char* title) {
  static const char closebtn[14][16] = {
    "OOOOOOOOOOOOOOO@",
    "OQQQQQQQQQQQQQ$@",
    "OQQQQQQQQQQQQQ$@",
    "OQQQ@@QQQQ@@QQ$@",
    "OQQQQ@@QQ@@QQQ$@",
    "OQQQQQ@@@@QQQQ$@",
    "OQQQQQQ@@QQQQQ$@",
    "OQQQQQ@@@@QQQQ$@",
    "OQQQQ@@QQ@@QQQ$@",
    "OQQQ@@QQQQ@@QQ$@",
    "OQQQQQQQQQQQQQ$@",
    "OQQQQQQQQQQQQQ$@",
    "O$$$$$$$$$$$$$$@",
    "@@@@@@@@@@@@@@@@",
  };
  boxfill8(buf, xsize, COL8_GRAY, 0, 0, xsize, 1);
  boxfill8(buf, xsize, COL8_WHITE, 1, 1, xsize - 1, 2);
  boxfill8(buf, xsize, COL8_GRAY, 0, 0, 1, ysize);
  boxfill8(buf, xsize, COL8_WHITE, 1, 1, 2, ysize - 1);
  boxfill8(buf, xsize, COL8_DARK_GRAY, xsize - 2, 1, xsize - 1, ysize - 1);
  boxfill8(buf, xsize, COL8_BLACK, xsize - 1, 0, xsize, ysize);
  boxfill8(buf, xsize, COL8_GRAY, 2, 2, xsize - 2, ysize - 2);
  boxfill8(buf, xsize, COL8_DARK_BLUE, 3, 3, xsize - 3, 21);
  boxfill8(buf, xsize, COL8_DARK_GRAY, 1, ysize - 2, xsize - 1, ysize - 1);
  boxfill8(buf, xsize, COL8_BLACK, 0, ysize - 1, xsize, ysize);
  putfonts8_asc(buf, xsize, 24, 4, COL8_WHITE, title);
  for (int y = 0; y < 14; ++y) {
    for (int x = 0; x < 16; ++x) {
      unsigned char c;
      switch (closebtn[y][x]) {
      case '@':  c = COL8_BLACK; break;
      case '$':  c = COL8_DARK_GRAY; break;
      case 'Q':  c = COL8_GRAY; break;
      default:   c = COL8_WHITE; break;
      }
      buf[(5 + y) * xsize + (xsize - 21 + x)] = c;
    }
  }
}

void HariMain(void) {
  init_gdtidt();
  init_pic();
  io_sti();  // Enable CPU interrupt after IDT/PIC initialization.

  unsigned char keybuf[32], mousebuf[128];
  MOUSE_DEC mdec;
  fifo8_init(&keyfifo, 32, keybuf);
  fifo8_init(&mousefifo, 128, mousebuf);
  init_pit();
  io_out8(PIC0_IMR, 0xf8);  // Enable PIT, PIC1 and keyboard.
  io_out8(PIC1_IMR, 0xef);  // Enable mouse.

  FIFO8 timerfifo;
  unsigned char timerbuf[8];
  fifo8_init(&timerfifo, 8, timerbuf);
  TIMER* timer[3];
  timer[0] = timer_alloc();
  timer_init(timer[0], &timerfifo, 1);
  timer_settime(timer[0], 1000);  // 10 sec
  timer[1] = timer_alloc();
  timer_init(timer[1], &timerfifo, 2);
  timer_settime(timer[1], 300);  // 3 sec
  timer[2] = timer_alloc();
  timer_init(timer[2], &timerfifo, 3);
  timer_settime(timer[2], 50);  // 0.5 sec

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
  SHEET* sht_win = sheet_alloc(shtctl);
  unsigned char* buf_back = (unsigned char*)memman_alloc_4k(memman, binfo->scrnx * binfo->scrny);
  sheet_setbuf(sht_back, buf_back, binfo->scrnx, binfo->scrny, -1);
  unsigned char buf_mouse[16 * 16];
  sheet_setbuf(sht_mouse, buf_mouse, 16, 16, 99);
  unsigned char* buf_win = (unsigned char*)memman_alloc_4k(memman, 160 * 68);
  sheet_setbuf(sht_win, buf_win, 160, 52, -1);
  init_screen8(buf_back, binfo->scrnx, binfo->scrny);

  init_mouse_cursor8(buf_mouse, 99);
  make_window8(buf_win, 160, 52, "counter");
  int mx = (binfo->scrnx - 16) / 2;
  int my = (binfo->scrny - 28 - 16) / 2;
  sheet_slide(shtctl, sht_back, 0, 0);
  sheet_slide(shtctl, sht_mouse, mx, my);
  sheet_slide(shtctl, sht_win, 80, 72);
  sheet_updown(shtctl, sht_back, 0);
  sheet_updown(shtctl, sht_win, 1);
  sheet_updown(shtctl, sht_mouse, 2);

  char s[40];
  sprintf(s, "(%3d, %3d)", mx, my);
  putfonts8_asc(buf_back, binfo->scrnx, 0, 0, COL8_WHITE, s);

  sprintf(s, "memory %dMB   free : %dKB",
          memtotal / (1024 * 1024), memman_total(memman) / 1024);
  putfonts8_asc(buf_back, binfo->scrnx, 0, 32, COL8_WHITE, s);

  sheet_refresh(shtctl, sht_back, 0, 0, binfo->scrnx, 48);

  for (;;) {
    io_cli();
    sprintf(s, "%09d", timerctl.count);
    boxfill8(buf_win, 160, COL8_GRAY, 40, 28, 120, 44);
    putfonts8_asc(buf_win, 160, 40, 28, COL8_BLACK, s);
    sheet_refresh(shtctl, sht_win, 40, 28, 120, 44);

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
    if (fifo8_status(&timerfifo) != 0) {
      int tid = fifo8_get(&timerfifo);
      io_sti();
      switch (tid) {
      case 1:  // 10sec
        putfonts8_asc(buf_back, binfo->scrnx, 0, 64, COL8_WHITE, "10[sec]");
        sheet_refresh(shtctl, sht_back, 0, 64, 56, 80);
        break;
      case 2:  // 3sec
        putfonts8_asc(buf_back, binfo->scrnx, 0, 80, COL8_WHITE, "3[sec]");
        sheet_refresh(shtctl, sht_back, 0, 80, 48, 96);
        break;
      case 3:  // 0.5sec
      case 4:  // 0.5sec
        {
          unsigned char col = tid == 3 ? COL8_WHITE : COL8_DARK_GRAY;
          boxfill8(buf_back, binfo->scrnx, col, 8, 96, 16, 112);
          sheet_refresh(shtctl, sht_back, 8, 96, 16, 112);
          timer_init(timer[2], &timerfifo, (3 + 4) - tid);
          timer_settime(timer[2], 50);
        }
        break;
      }
      continue;
    }

    io_stihlt();
  }
}
