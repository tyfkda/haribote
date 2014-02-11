#include "bootpack.h"
#include "dsctbl.h"
#include "fifo.h"
#include "graphics.h"
#include "int.h"
#include "keyboard.h"
#include "memory.h"
#include "mouse.h"
#include "mtask.h"
#include "sheet.h"
#include "stdio.h"
#include "timer.h"

void putfonts8_asc_sht(SHTCTL* shtctl, SHEET* sht, int x, int y, int c, int b, const char* s, int l) {
  boxfill8(sht->buf, sht->bxsize, b, x, y, x + l * 8, y + 16);
  putfonts8_asc(sht->buf, sht->bxsize, x, y, c, s);
  sheet_refresh(shtctl, sht, x, y, x + l * 8, y + 16);
}

void make_window8(unsigned char* buf, int xsize, int ysize, char* title, char act) {
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
  unsigned char tc, tbc;
  if (act) {
    tc = COL8_WHITE;
    tbc = COL8_DARK_BLUE;
  } else {
    tc = COL8_GRAY;
    tbc = COL8_DARK_GRAY;
  }
  boxfill8(buf, xsize, COL8_GRAY, 0, 0, xsize, 1);
  boxfill8(buf, xsize, COL8_WHITE, 1, 1, xsize - 1, 2);
  boxfill8(buf, xsize, COL8_GRAY, 0, 0, 1, ysize);
  boxfill8(buf, xsize, COL8_WHITE, 1, 1, 2, ysize - 1);
  boxfill8(buf, xsize, COL8_DARK_GRAY, xsize - 2, 1, xsize - 1, ysize - 1);
  boxfill8(buf, xsize, COL8_BLACK, xsize - 1, 0, xsize, ysize);
  boxfill8(buf, xsize, COL8_GRAY, 2, 2, xsize - 2, ysize - 2);
  boxfill8(buf, xsize, tbc, 3, 3, xsize - 3, 21);
  boxfill8(buf, xsize, COL8_DARK_GRAY, 1, ysize - 2, xsize - 1, ysize - 1);
  boxfill8(buf, xsize, COL8_BLACK, 0, ysize - 1, xsize, ysize);
  putfonts8_asc(buf, xsize, 24, 4, tc, title);
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

void make_textbox8(SHEET* sht, int x0, int y0, int sx, int sy, int c) {
  int x1 = x0 + sx, y1 = y0 + sy;
  boxfill8(sht->buf, sht->bxsize, COL8_GRAY, x0 - 2, y0 - 3, x1 + 2, y0 - 2);
  boxfill8(sht->buf, sht->bxsize, COL8_GRAY, x0 - 3, y0 - 3, x0 - 2, y1 + 2);
  boxfill8(sht->buf, sht->bxsize, COL8_WHITE, x0 - 3, y1 + 2, x1 + 2, y1 + 3);
  boxfill8(sht->buf, sht->bxsize, COL8_WHITE, x1 + 2, y0 - 3, x1 + 3, y1 + 3);
  boxfill8(sht->buf, sht->bxsize, COL8_BLACK, x0 - 1, y0 - 2, x1 + 1, y0 - 1);
  boxfill8(sht->buf, sht->bxsize, COL8_BLACK, x0 - 2, y0 - 2, x0 - 1, y1 + 1);
  boxfill8(sht->buf, sht->bxsize, COL8_GRAY, x0 - 2, y1 + 1, x1 + 1, y1 + 2);
  boxfill8(sht->buf, sht->bxsize, COL8_GRAY, x1 + 1, y0 - 2, x1 + 2, y1 + 2);
  boxfill8(sht->buf, sht->bxsize, c, x0 - 1, y0 - 1, x1 + 1, y1 + 1);
}

void task_b_main(SHTCTL* shtctl, SHEET* sht_win_b) {
  FIFO fifo;
  int fifobuf[128];
  fifo_init(&fifo, 128, fifobuf, NULL);

  TIMER* timer_put = timer_alloc();
  timer_init(timer_put, &fifo, 1);
  timer_settime(timer_put, 1);

  int count = 0;
  for (;;) {
    ++count;
    io_cli();
    if (fifo_status(&fifo) == 0) {
      io_stihlt();
      //io_sti();
      continue;
    }
    int i = fifo_get(&fifo);
    io_sti();
    switch (i) {
    case 1:
      {
        char s[16];
        sprintf(s, "%9d", count);
        putfonts8_asc_sht(shtctl, sht_win_b, 24, 28, COL8_BLACK, COL8_GRAY, s, 9);
        timer_settime(timer_put, 1);
      }
      break;
    }
  }
}

void HariMain(void) {
  init_gdtidt();
  init_pic();
  io_sti();  // Enable CPU interrupt after IDT/PIC initialization.

  FIFO fifo;
  int fifobuf[128];
  MOUSE_DEC mdec;
  fifo_init(&fifo, 128, fifobuf, NULL);
  init_pit();
  init_keyboard(&fifo, 256);
  enable_mouse(&fifo, 512, &mdec);
  io_out8(PIC0_IMR, 0xf8);  // Enable PIT, PIC1 and keyboard.
  io_out8(PIC1_IMR, 0xef);  // Enable mouse.

  unsigned int memtotal = memtest(0x00400000, 0xbfffffff);
  MEMMAN *memman = (MEMMAN*)MEMMAN_ADDR;
  memman_init(memman);
  memman_free(memman, 0x00001000, 0x0009e000); /* 0x00001000 - 0x0009efff */
  memman_free(memman, 0x00400000, memtotal - 0x00400000);

  BOOTINFO* binfo = (BOOTINFO*)ADR_BOOTINFO;
  init_palette();
  SHTCTL* shtctl = shtctl_init(memman, binfo->vram, binfo->scrnx, binfo->scrny);

  TASK* task_a = task_init(memman);
  fifo.task = task_a;

  // sht_back
  SHEET* sht_back = sheet_alloc(shtctl);
  unsigned char* buf_back = (unsigned char*)memman_alloc_4k(memman, binfo->scrnx * binfo->scrny);
  sheet_setbuf(sht_back, buf_back, binfo->scrnx, binfo->scrny, -1);
  init_screen8(buf_back, binfo->scrnx, binfo->scrny);

  char s[40];
  // sht_win_b
  SHEET* sht_win_b[3];
  TASK* task_b[3];
  for (int i = 0; i < 3; ++i) {
    sht_win_b[i] = sheet_alloc(shtctl);
    unsigned char* buf_win_b = (unsigned char*)memman_alloc_4k(memman, 144 * 52);
    sheet_setbuf(sht_win_b[i], buf_win_b, 144, 52, -1);
    sprintf(s, "task_b%d", i);
    make_window8(buf_win_b, 144, 52, s, FALSE);

    TASK* task = task_b[i] = task_alloc();
    task->tss.esp = memman_alloc_4k(memman, 64 * 1024) + 64 * 1024 - 4 - 4 * 2;
    task->tss.eip = (int) &task_b_main;
    task->tss.cs = 2 * 8;
    task->tss.es = task->tss.ss = task->tss.ds = task->tss.fs = task->tss.gs = 1 * 8;
    *((int*)(task->tss.esp + 4)) = (int)shtctl;
    *((int*)(task->tss.esp + 8)) = (int)sht_win_b[i];
    task_run(task);
  }

  // sht_win
  SHEET* sht_win = sheet_alloc(shtctl);
  unsigned char* buf_win = (unsigned char*)memman_alloc_4k(memman, 160 * 52);
  sheet_setbuf(sht_win, buf_win, 160, 52, -1);
  make_window8(buf_win, 160, 52, "window", TRUE);
  make_textbox8(sht_win, 8, 28, 144, 16, COL8_WHITE);
  int cursor_x = 8;
  int cursor_c = COL8_WHITE;
  TIMER* timer;
  timer = timer_alloc();
  timer_init(timer, &fifo, 0);
  timer_settime(timer, 50);  // 0.5 sec

  // sht_mouse
  SHEET* sht_mouse = sheet_alloc(shtctl);
  unsigned char buf_mouse[16 * 16];
  sheet_setbuf(sht_mouse, buf_mouse, 16, 16, 99);
  init_mouse_cursor8(buf_mouse, 99);
  int mx = (binfo->scrnx - 16) / 2;
  int my = (binfo->scrny - 28 - 16) / 2;

  sheet_slide(shtctl, sht_back, 0, 0);
  sheet_slide(shtctl, sht_win_b[0], 168,  56);
  sheet_slide(shtctl, sht_win_b[1],   8, 116);
  sheet_slide(shtctl, sht_win_b[2], 168, 116);
  sheet_slide(shtctl, sht_win, 80, 72);
  sheet_slide(shtctl, sht_mouse, mx, my);
  sheet_updown(shtctl, sht_back, 0);
  sheet_updown(shtctl, sht_win_b[0], 1);
  sheet_updown(shtctl, sht_win_b[1], 2);
  sheet_updown(shtctl, sht_win_b[2], 3);
  sheet_updown(shtctl, sht_win, 4);
  sheet_updown(shtctl, sht_mouse, 5);

  sprintf(s, "(%3d, %3d)", mx, my);
  putfonts8_asc_sht(shtctl, sht_back, 0, 0, COL8_WHITE, COL8_DARK_CYAN, s, 10);

  sprintf(s, "memory %dMB   free : %dKB",
          memtotal / (1024 * 1024), memman_total(memman) / 1024);
  putfonts8_asc_sht(shtctl, sht_back, 0, 32, COL8_WHITE, COL8_DARK_CYAN, s, 40);

  for (;;) {
    io_cli();

    if (fifo_status(&fifo) == 0) {
      task_sleep(task_a);
      io_sti();
      continue;
    }

    int i = fifo_get(&fifo);
    io_sti();
    if (256 <= i && i < 512) {  // Keyboard data.
      char s[4];
      sprintf(s, "%02X", i - 256);
      putfonts8_asc_sht(shtctl, sht_back, 0, 16, COL8_WHITE, COL8_DARK_CYAN, s, 2);
      static const char keytable[0x54] = {
        0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '^', 0, 0,
        'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '@', '[', 0, 0, 'A', 'S',
        'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', ':', 0, 0, ']', 'Z', 'X', 'C', 'V',
        'B', 'N', 'M', ',', '.', '/', 0, '*', 0, ' ', 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, '7', '8', '9', '-', '4', '5', '6', '+', '1',
        '2', '3', '0', '.',
      };
      if (i < 256 + 0x54) {
        if (keytable[i - 256] != 0) {
          s[0] = keytable[i - 256];
          s[1] = '\0';
          putfonts8_asc_sht(shtctl, sht_win, cursor_x, 28, COL8_BLACK, COL8_WHITE, s, 1);
          cursor_x += 8;
        }
      }
      if (i == 256 + 0x0e && cursor_x > 8) {
        putfonts8_asc_sht(shtctl, sht_win, cursor_x, 28, COL8_BLACK, COL8_WHITE, " ", 1);
        cursor_x -= 8;
      }
      boxfill8(sht_win->buf, sht_win->bxsize, cursor_c, cursor_x, 28, cursor_x + 8, 44);
      sheet_refresh(shtctl, sht_win, cursor_x, 28, cursor_x + 8, 44);
      continue;
    } else if (512 <= i && i < 768) {  // Mouse data.
      if (mouse_decode(&mdec, i - 512) != 0) {
        // Erase mouse cursor.
        sprintf(s, "[lcr %4d %4d]", mdec.x, mdec.y);
        if ((mdec.btn & 0x01) != 0)
          s[1] = 'L';
        if ((mdec.btn & 0x02) != 0)
          s[1] = 'R';
        if ((mdec.btn & 0x04) != 0)
          s[1] = 'C';
        putfonts8_asc_sht(shtctl, sht_back, 32, 16, COL8_WHITE, COL8_DARK_CYAN, s, 15);

        // Move mouse cursor.
        mx += mdec.x;
        my += mdec.y;
        if (mx < 0)  mx = 0;
        if (my < 0)  my = 0;
        if (mx >= binfo->scrnx - 1)  mx = binfo->scrnx - 1;
        if (my >= binfo->scrny - 1)  my = binfo->scrny - 1;
        sprintf(s, "(%3d, %3d)", mx, my);
        putfonts8_asc_sht(shtctl, sht_back, 0, 0, COL8_WHITE, COL8_DARK_CYAN, s, 10);
        sheet_slide(shtctl, sht_mouse, mx, my);

        if ((mdec.btn & 0x01) != 0)
          sheet_slide(shtctl, sht_win, mx - 80, my - 8);
      }
      continue;
    }
    switch (i) {
    case 0:  // 0.5sec
    case 1:  // 0.5sec
      {
        cursor_c = i == 0 ? COL8_WHITE : COL8_BLACK;
        boxfill8(sht_win->buf, sht_win->bxsize, cursor_c, cursor_x, 28, cursor_x + 8, 44);
        sheet_refresh(shtctl, sht_win, cursor_x, 28, cursor_x + 8, 44);
        timer_init(timer, &fifo, 1 - i);
        timer_settime(timer, 50);
      }
      break;
    }
    continue;
  }
}
