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

void make_wtitle8(unsigned char* buf, int xsize, char* title, char act) {
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
  boxfill8(buf, xsize, tbc, 3, 3, xsize - 3, 21);
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

void make_window8(unsigned char* buf, int xsize, int ysize, char* title, char act) {
  boxfill8(buf, xsize, COL8_GRAY, 0, 0, xsize, 1);
  boxfill8(buf, xsize, COL8_WHITE, 1, 1, xsize - 1, 2);
  boxfill8(buf, xsize, COL8_GRAY, 0, 0, 1, ysize);
  boxfill8(buf, xsize, COL8_WHITE, 1, 1, 2, ysize - 1);
  boxfill8(buf, xsize, COL8_DARK_GRAY, xsize - 2, 1, xsize - 1, ysize - 1);
  boxfill8(buf, xsize, COL8_BLACK, xsize - 1, 0, xsize, ysize);
  boxfill8(buf, xsize, COL8_GRAY, 2, 2, xsize - 2, ysize - 2);
  boxfill8(buf, xsize, COL8_DARK_GRAY, 1, ysize - 2, xsize - 1, ysize - 1);
  boxfill8(buf, xsize, COL8_BLACK, 0, ysize - 1, xsize, ysize);
  make_wtitle8(buf, xsize, title, act);
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

void console_task(SHTCTL* shtctl, SHEET* sheet) {
  TASK* task = task_now();
  int fifobuf[128];
  fifo_init(&task->fifo, 128, fifobuf, task);

  TIMER* timer = timer_alloc();
  timer_init(timer, &task->fifo, 1);
  timer_settime(timer, 50);

  int cursor_x = 16, cursor_c = COL8_BLACK;
  // Show prompt.
  putfonts8_asc_sht(shtctl, sheet, 8, 28, COL8_WHITE, COL8_BLACK, ">", 1);

  for (;;) {
    io_cli();
    if (fifo_status(&task->fifo) == 0) {
      task_sleep(task);
      continue;
    }
    int i = fifo_get(&task->fifo);
    io_sti();
    if (256 <= i && i < 512) {  // Keyboard data (from task A).
      if (i == 8 + 256) {  // Back space.
        if (cursor_x > 16) {
          putfonts8_asc_sht(shtctl, sheet, cursor_x, 28, COL8_WHITE, COL8_BLACK, " ", 1);
          cursor_x -= 8;
        }
      } else {  // Normal character.
        if (cursor_x < 240) {
          char s[] = { i - 256, '\0' };
          putfonts8_asc_sht(shtctl, sheet, cursor_x, 28, COL8_WHITE, COL8_BLACK, s, 1);
          cursor_x += 8;
        }
      }
    } else {
      switch (i) {
      case 0:
      case 1:
        cursor_c = i == 0 ? COL8_WHITE : COL8_BLACK;
        timer_init(timer, &task->fifo, 1 - i);
        timer_settime(timer, 50);
        break;
      }
    }
    // Redraw cursor.
    boxfill8(sheet->buf, sheet->bxsize, cursor_c, cursor_x, 28, cursor_x + 8, 44);
    sheet_refresh(shtctl, sheet, cursor_x, 28, cursor_x + 8, 44);
  }
}

static const char keytable[2][0x80] = {
  {  // Normal.
      0,   0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '^',   0,   0,
    'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '@', '[',   0,   0, 'A', 'S',
    'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', ':',   0,   0, ']', 'Z', 'X', 'C', 'V',
    'B', 'N', 'M', ',', '.', '/',   0, '*',   0, ' ',   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0, '7', '8', '9', '-', '4', '5', '6', '+', '1',
    '2', '3', '0', '.',   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,0x5c,   0,   0,   0,   0,   0,   0,   0,   0,   0,0x5c,   0,   0,
  },
  {  // Shift.
      0,   0, '!', '"', '#', '$', '%', '&', '*', '(', ')',   0, '=', '~',   0,   0,
    'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '`', '{',   0,   0, 'A', 'S',
    'D', 'F', 'G', 'H', 'J', 'K', 'L', '+', '*',   0,   0, '}', 'Z', 'X', 'C', 'V',
    'B', 'N', 'M', '<', '>', '?',   0, '*',   0, ' ',   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0, '7', '8', '9', '-', '4', '5', '6', '+', '1',
    '2', '3', '0', '.',   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0, '_',   0,   0,   0,   0,   0,   0,   0,   0,   0, '|',   0,   0,
  },
};

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

  init_palette();
  BOOTINFO* binfo = (BOOTINFO*)ADR_BOOTINFO;
  SHTCTL* shtctl = shtctl_init(memman, binfo->vram, binfo->scrnx, binfo->scrny);

  TASK* task_a = task_init(memman);
  fifo.task = task_a;
  task_run(task_a, 1, 2);

  // sht_back
  SHEET* sht_back = sheet_alloc(shtctl);
  unsigned char* buf_back = (unsigned char*)memman_alloc_4k(memman, binfo->scrnx * binfo->scrny);
  sheet_setbuf(sht_back, buf_back, binfo->scrnx, binfo->scrny, -1);
  init_screen8(buf_back, binfo->scrnx, binfo->scrny);

  char s[40];
  // sht_cons
  SHEET* sht_cons = sheet_alloc(shtctl);
  unsigned char* buf_cons = (unsigned char*)memman_alloc_4k(memman, 256 * 165);
  sheet_setbuf(sht_cons, buf_cons, 256, 165, -1);
  make_window8(buf_cons, 256, 165, "console", FALSE);
  make_textbox8(sht_cons, 8, 28, 240, 128, COL8_BLACK);
  TASK* task_cons = task_alloc();
  task_cons->tss.esp = memman_alloc_4k(memman, 64 * 1024) + 64 * 1024 - 4 - 4 * 2;
  task_cons->tss.eip = (int) &console_task;
  task_cons->tss.cs = 2 * 8;
  task_cons->tss.es = task_cons->tss.ss = task_cons->tss.ds = task_cons->tss.fs = task_cons->tss.gs = 1 * 8;
  *((int*)(task_cons->tss.esp + 4)) = (int)shtctl;
  *((int*)(task_cons->tss.esp + 8)) = (int)sht_cons;
  task_run(task_cons, 2, 2);

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
  sheet_slide(shtctl, sht_cons, 32, 4);
  sheet_slide(shtctl, sht_win, 80, 72);
  sheet_slide(shtctl, sht_mouse, mx, my);
  sheet_updown(shtctl, sht_back, 0);
  sheet_updown(shtctl, sht_cons, 1);
  sheet_updown(shtctl, sht_win, 2);
  sheet_updown(shtctl, sht_mouse, 3);

  sprintf(s, "(%3d, %3d)", mx, my);
  putfonts8_asc_sht(shtctl, sht_back, 0, 0, COL8_WHITE, COL8_DARK_CYAN, s, 10);

  sprintf(s, "memory %dMB   free : %dKB",
          memtotal / (1024 * 1024), memman_total(memman) / 1024);
  putfonts8_asc_sht(shtctl, sht_back, 0, 32, COL8_WHITE, COL8_DARK_CYAN, s, 40);

  int key_to = 0, key_shift = 0;

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
      switch (i) {
      case 0x0e + 256:  // Back space.
        if (key_to == 0) {  // To task A
          if (cursor_x > 8) {
            putfonts8_asc_sht(shtctl, sht_win, cursor_x, 28, COL8_BLACK, COL8_WHITE, " ", 1);
            cursor_x -= 8;
          }
        } else {  // To console.
          fifo_put(&task_cons->fifo, 8 + 256);
        }
        break;
      case 0x0f + 256:  // Tab.
        if (key_to == 0) {
          key_to = 1;
          make_wtitle8(buf_win, sht_win->bxsize, "task_a", FALSE);
          make_wtitle8(buf_cons, sht_cons->bxsize, "console", TRUE);
        } else {
          key_to = 0;
          make_wtitle8(buf_win, sht_win->bxsize, "task_a", TRUE);
          make_wtitle8(buf_cons, sht_cons->bxsize, "console", FALSE);
        }
        sheet_refresh(shtctl, sht_win, 0, 0, sht_win->bxsize, 21);
        sheet_refresh(shtctl, sht_cons, 0, 0, sht_cons->bxsize, 21);
        break;
      case 0x2a + 256:  // Left shift on.
        key_shift |= 1;
        break;
      case 0x36 + 256:  // Left shift on.
        key_shift |= 2;
        break;
      case 0xaa + 256:  // Left shift on.
        key_shift &= ~1;
        break;
      case 0xb6 + 256:  // Left shift on.
        key_shift &= ~2;
        break;
      default:
        if (i < 256 + 0x80) {  // Normal character.
          s[0] = keytable[key_shift ? 1 : 0][i - 256];
          if (s[0] != 0) {  // Normal character.
            if ('A' <= s[0] && s[0] <= 'Z' && !key_shift)
              s[0] += 'a' - 'A';
            if (key_to == 0) {  // To task A
              if (cursor_x < 128) {
                s[1] = '\0';
                putfonts8_asc_sht(shtctl, sht_win, cursor_x, 28, COL8_BLACK, COL8_WHITE, s, 1);
                cursor_x += 8;
              }
            } else {  // To console.
              fifo_put(&task_cons->fifo, s[0] + 256);
            }
          }
        }
        break;
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
      cursor_c = i == 0 ? COL8_WHITE : COL8_BLACK;
      boxfill8(sht_win->buf, sht_win->bxsize, cursor_c, cursor_x, 28, cursor_x + 8, 44);
      sheet_refresh(shtctl, sht_win, cursor_x, 28, cursor_x + 8, 44);
      timer_init(timer, &fifo, 1 - i);
      timer_settime(timer, 50);
      break;
    }
    continue;
  }
}
