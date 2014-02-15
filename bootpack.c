#include "bootpack.h"
#include "console.h"
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
#include "window.h"

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
  *((int*)0x0fe4) = (int)shtctl;

  TASK* task_a = task_init(memman);
  fifo.task = task_a;
  task_run(task_a, 1, 2);

  // sht_back
  SHEET* sht_back = sheet_alloc(shtctl);
  unsigned char* buf_back = (unsigned char*)memman_alloc_4k(memman, binfo->scrnx * binfo->scrny);
  sheet_setbuf(sht_back, buf_back, binfo->scrnx, binfo->scrny, -1);
  init_screen8(buf_back, binfo->scrnx, binfo->scrny);

  // sht_cons
  SHEET* sht_cons = sheet_alloc(shtctl);
  unsigned char* buf_cons = (unsigned char*)memman_alloc_4k(memman, 256 * 165);
  sheet_setbuf(sht_cons, buf_cons, 256, 165, -1);
  make_window8(buf_cons, 256, 165, "console", FALSE);
  make_textbox8(sht_cons, 8, 28, 240, 128, COL8_BLACK);
  TASK* task_cons = task_alloc();
  task_cons->tss.esp = memman_alloc_4k(memman, 64 * 1024) + 64 * 1024 - 4 - 4 * 3;
  task_cons->tss.eip = (int) &console_task;
  task_cons->tss.cs = 2 * 8;
  task_cons->tss.es = task_cons->tss.ss = task_cons->tss.ds = task_cons->tss.fs = task_cons->tss.gs = 1 * 8;
  *((int*)(task_cons->tss.esp + 4)) = (int)shtctl;
  *((int*)(task_cons->tss.esp + 8)) = (int)sht_cons;
  *((int*)(task_cons->tss.esp + 12)) = (int)memtotal;
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
  sheet_slide(shtctl, sht_win, 32, 170);
  sheet_slide(shtctl, sht_mouse, mx, my);
  sheet_updown(shtctl, sht_back, 0);
  sheet_updown(shtctl, sht_cons, 1);
  sheet_updown(shtctl, sht_win, 2);
  sheet_updown(shtctl, sht_mouse, 3);

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
      switch (i) {
      case 0x1c + 256:  // Enter.
        if (key_to != 0)  // To console.
          fifo_put(&task_cons->fifo, 10 + 256);
        break;
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
          cursor_c = -1;  // Hide cursor.
          boxfill8(sht_win->buf, sht_win->bxsize, COL8_WHITE, cursor_x, 28, cursor_x + 8, 44);
          fifo_put(&task_cons->fifo, 2);  // Show console cursor.
        } else {
          key_to = 0;
          make_wtitle8(buf_win, sht_win->bxsize, "task_a", TRUE);
          make_wtitle8(buf_cons, sht_cons->bxsize, "console", FALSE);
          cursor_c = COL8_BLACK;  // Show cursor.
          fifo_put(&task_cons->fifo, 3);  // Hide console cursor.
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
      case 0x3b + 256:  // F1
        if (key_shift != 0 && task_cons->tss.ss0 != 0) {  // Shift + F1
          CONSOLE* cons = (CONSOLE*)*((int*)0x0fec);
          cons_putstr0(cons, "\nBreak(key) :\n");
          io_cli();
          task_cons->tss.eax = (int)&(task_cons->tss.esp0);
          task_cons->tss.eip = (int)asm_end_app;
          io_sti();
        }
        break;
      default:
        if (i < 256 + 0x80) {  // Normal character.
          char s[2];
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
      if (cursor_c >= 0) {
        boxfill8(sht_win->buf, sht_win->bxsize, cursor_c, cursor_x, 28, cursor_x + 8, 44);
      }
      sheet_refresh(shtctl, sht_win, cursor_x, 28, cursor_x + 8, 44);
      continue;
    } else if (512 <= i && i < 768) {  // Mouse data.
      if (mouse_decode(&mdec, i - 512) != 0) {
        // Move mouse cursor.
        mx += mdec.x;
        my += mdec.y;
        if (mx < 0)  mx = 0;
        if (my < 0)  my = 0;
        if (mx >= binfo->scrnx - 1)  mx = binfo->scrnx - 1;
        if (my >= binfo->scrny - 1)  my = binfo->scrny - 1;
        sheet_slide(shtctl, sht_mouse, mx, my);

        if ((mdec.btn & 0x01) != 0)
          sheet_slide(shtctl, sht_win, mx - 80, my - 8);
      }
      continue;
    }
    switch (i) {
    case 0:  // 0.5sec
    case 1:  // 0.5sec
      if (cursor_c >= 0) {
        cursor_c = i == 0 ? COL8_WHITE : COL8_BLACK;
        boxfill8(sht_win->buf, sht_win->bxsize, cursor_c, cursor_x, 28, cursor_x + 8, 44);
        sheet_refresh(shtctl, sht_win, cursor_x, 28, cursor_x + 8, 44);
      }
      timer_init(timer, &fifo, 1 - i);
      timer_settime(timer, 50);
      break;
    }
    continue;
  }
}
