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

static void keywin_off(SHTCTL* shtctl, SHEET* key_win) {
  change_wtitle8(shtctl, key_win, FALSE);
  if ((key_win->flags & 0x20) != 0)
    fifo_put(&key_win->task->fifo, 3);  // Send hide cursor message.
}

static void keywin_on(SHTCTL* shtctl, SHEET* key_win) {
  change_wtitle8(shtctl, key_win, TRUE);
  if ((key_win->flags & 0x20) != 0)
    fifo_put(&key_win->task->fifo, 2);  // Send show cursor message.
}

static const char keytable[2][0x80] = {
  {  // Normal.
      0,   0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '^',0x08,   0,
    'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '@', '[',0x0a,   0, 'A', 'S',
    'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', ':',   0,   0, ']', 'Z', 'X', 'C', 'V',
    'B', 'N', 'M', ',', '.', '/',   0, '*',   0, ' ',   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0, '7', '8', '9', '-', '4', '5', '6', '+', '1',
    '2', '3', '0', '.',   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,0x5c,   0,   0,   0,   0,   0,   0,   0,   0,   0,0x5c,   0,   0,
  },
  {  // Shift.
      0,   0, '!', '"', '#', '$', '%', '&', '*', '(', ')',   0, '=', '~',0x08,   0,
    'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '`', '{',0x0a,   0, 'A', 'S',
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
  memman_free(memman, (void*)0x00001000, 0x0009e000); /* 0x00001000 - 0x0009efff */
  memman_free(memman, (void*)0x00400000, memtotal - 0x00400000);

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
  SHEET* sht_cons[2];
  unsigned char* buf_cons[2];
  TASK* task_cons[2];
  int* cons_fifo[2];
  for (int i = 0; i < 2; ++i) {
    sht_cons[i] = sheet_alloc(shtctl);
    buf_cons[i] = (unsigned char*)memman_alloc_4k(memman, 256 * 165);
    sheet_setbuf(sht_cons[i], buf_cons[i], 256, 165, -1);
    make_window8(buf_cons[i], 256, 165, "console", FALSE);
    make_textbox8(sht_cons[i], 8, 28, 240, 128, COL8_BLACK);
    task_cons[i] = task_alloc();
    task_cons[i]->tss.esp = (int)memman_alloc_4k(memman, 64 * 1024) + 64 * 1024 - 4 - 4 * 3;
    task_cons[i]->tss.eip = (int) &console_task;
    task_cons[i]->tss.cs = 2 * 8;
    task_cons[i]->tss.es = task_cons[i]->tss.ss = task_cons[i]->tss.ds = task_cons[i]->tss.fs = task_cons[i]->tss.gs = 1 * 8;
    *((int*)(task_cons[i]->tss.esp + 4)) = (int)shtctl;
    *((int*)(task_cons[i]->tss.esp + 8)) = (int)sht_cons[i];
    *((int*)(task_cons[i]->tss.esp + 12)) = (int)memtotal;
    task_run(task_cons[i], 2, 2);
    sht_cons[i]->task = task_cons[i];
    sht_cons[i]->flags |= 0x20;

    cons_fifo[i] = (int*)memman_alloc_4k(memman, 128 * sizeof(int));
    fifo_init(&task_cons[i]->fifo, 128, cons_fifo[i], task_cons[i]);
  }

  // sht_mouse
  SHEET* sht_mouse = sheet_alloc(shtctl);
  unsigned char buf_mouse[16 * 16];
  sheet_setbuf(sht_mouse, buf_mouse, 16, 16, 99);
  init_mouse_cursor8(buf_mouse, 99);
  int mx = (binfo->scrnx - 16) / 2;
  int my = (binfo->scrny - 28 - 16) / 2;
  int mmx = -1, mmy = -1;  // Mouse drag position.

  sheet_slide(shtctl, sht_back, 0, 0);
  sheet_slide(shtctl, sht_cons[1], 264, 2);
  sheet_slide(shtctl, sht_cons[0], 8, 2);
  sheet_slide(shtctl, sht_mouse, mx, my);
  sheet_updown(shtctl, sht_back, 0);
  sheet_updown(shtctl, sht_cons[1], 1);
  sheet_updown(shtctl, sht_cons[0], 2);
  sheet_updown(shtctl, sht_mouse, 3);

  int key_shift = 0;
  SHEET* key_win = sht_cons[0];
  keywin_on(shtctl, sht_cons[0]);

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
      char s[30];
      sprintf(s, "key:%02x", i - 256);
      putfonts8_asc_sht(shtctl, sht_back, 0, sht_back->bysize - 28, COL8_RED, COL8_DARK_GRAY, s, strlen(s));

      switch (i) {
      case 0x0f + 256:  // Tab.
        {
          keywin_off(shtctl, key_win);
          int j = key_win->height - 1;
          if (j == 0)
            j = shtctl->top - 1;
          key_win = shtctl->sheets[j];
          keywin_on(shtctl, key_win);
        }break;
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
        if (key_shift != 0) {  // Shift + F1
          TASK* task = key_win->task;
          if (task != NULL && task->tss.ss0 != 0) {
            CONSOLE* cons = task->cons;
            if (cons != NULL)
              cons_putstr0(cons, "\nBreak(key) :\n");
            io_cli();
            task->tss.eax = (int)&(task->tss.esp0);
            task->tss.eip = (int)asm_end_app;
            io_sti();
          }
        }
        break;
      case 0x57 + 256:  // F11
        // Move most bottom (except back!) sheet to the top.
        sheet_updown(shtctl, shtctl->sheets[1], shtctl->top - 1);
        break;
      default:
        if (i < 256 + 0x80) {  // Normal character.
          char s[2];
          s[0] = keytable[key_shift ? 1 : 0][i - 256];
          if (s[0] != 0) {  // Normal character.
            if ('A' <= s[0] && s[0] <= 'Z' && !key_shift)
              s[0] += 'a' - 'A';
            fifo_put(&key_win->task->fifo, s[0] + 256);
          }
        }
        break;
      }
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

        if ((mdec.btn & 0x01) != 0) {  // Mouse left button pressed.
          if (mmx < 0) {  // Normal mode.
            for (int j = shtctl->top; --j > 0; ) {
              SHEET* sht = shtctl->sheets[j];
              int x = mx - sht->vx0;
              int y = my - sht->vy0;
              if (x < 0 || x >= sht->bxsize || y < 0 || y >= sht->bysize ||
                  sht->buf[y * sht->bxsize + x] == sht->col_inv)
                continue;
              sheet_updown(shtctl, sht, shtctl->top - 1);
              if (sht->bxsize - 21 <= x && x <= sht->bxsize - 5 && 5 <= 5 && y < 19) {
                // Close button clicked.
                if ((sht->flags & 0x10) != 0) {  // Window created by application.
                  TASK* task = sht->task;
                  CONSOLE* cons = task->cons;
                  if (cons != NULL)
                    cons_putstr0(cons, "\nBreak(mouse) :\n");
                  io_cli();
                  task->tss.eax = (int)&(task->tss.esp0);
                  task->tss.eip = (int)asm_end_app;
                  io_sti();
                }
                break;
              }
              if (sht != key_win) {
                keywin_off(shtctl, key_win);
                key_win = sht;
                keywin_on(shtctl, key_win);
              }
              if (3 <= x && x < sht->bxsize - 3 && 3 <= y && y < 21) {
                mmx = mx;  // Go to drag mode.
                mmy = my;
              }
              break;
            }
          } else {  // Drag mode.
            SHEET* sht = shtctl->sheets[shtctl->top - 1];
            int x = mx - mmx;
            int y = my - mmy;
            sheet_slide(shtctl, sht, sht->vx0 + x, sht->vy0 + y);
            mmx = mx;
            mmy = my;
          }
        } else {
          mmx = -1;  // Go to normal mode.
        }
      }
      continue;
    }
  }
}
