#include "console.h"
#include "apilib.h"  // KEY
#include "bootpack.h"
#include "fd.h"
#include "graphics.h"
#include "mtask.h"
#include "sheet.h"
#include "stdio.h"
#include "string.h"
#include "timer.h"
#include "util.h"
#include "window.h"

#define X0     (8)
#define Y0     (8 + 20)
#define FONTW  (8)
#define FONTH  (16)
#define CONSOLE_NX  (40)
#define CONSOLE_NY  (25)
#define CONSOLE_WIDTH   (CONSOLE_NX * FONTW + 8 * 2)
#define CONSOLE_HEIGHT  (CONSOLE_NY * FONTH + 8 * 2 + 20)

#define CMDLINE_MAX  (255)

static void cons_newline(CONSOLE* cons, int* pcurX, int* pcurY) {
  *pcurX = 8;
  SHEET* sheet = cons->sheet;
  if (*pcurY < Y0 + (CONSOLE_NY - 1) * FONTH || sheet == NULL) {
    *pcurY += FONTH;
    return;
  }

  if (pcurY != &cons->cur_y)
    cons->cur_y -= FONTH;
  unsigned char* buf = sheet->buf;
  int bxsize = sheet->bxsize;
  // Scroll.
  for (int y = Y0; y < Y0 + CONSOLE_NY * FONTH; ++y)
    memcpy(&buf[y * bxsize + X0], &buf[(y + FONTH) * bxsize + X0], CONSOLE_NX * FONTW);
  // Erase last line.
  boxfill8(buf, bxsize, cons->bgColor, X0, Y0 + (CONSOLE_NY - 1) * FONTH, X0 + CONSOLE_NX * FONTW, Y0 + CONSOLE_NY * FONTH);
  sheet_refresh(cons->shtctl, sheet, X0, Y0, X0 + CONSOLE_NX * FONTW, Y0 + CONSOLE_NY * FONTH);
}

static void cons_cls(CONSOLE* cons) {
  SHEET* sheet = cons->sheet;
  boxfill8(sheet->buf, sheet->bxsize, cons->bgColor, X0, Y0, X0 + CONSOLE_NX * FONTW, Y0 + CONSOLE_NY * FONTH);
  sheet_refresh(cons->shtctl, sheet, X0, Y0, X0 + CONSOLE_NX * FONTW, Y0 + CONSOLE_NY * FONTH);
  cons->cur_x = X0;
  cons->cur_y = Y0;
}

void cons_putchar_at(CONSOLE* cons, int chr, char move, char neg, int* pcurX, int* pcurY) {
  unsigned char fontColor = cons->fontColor, backColor = cons->bgColor;
  if (neg)
    fontColor = cons->bgColor, backColor = cons->fontColor;
  char s[2] = { chr, '\0' };
  switch (chr) {
  case 0x09:  // Tab.
    for (;;) {
      if (cons->sheet != NULL)
        putfonts8_asc_sht(cons->shtctl, cons->sheet, *pcurX, *pcurY, fontColor, backColor, " ", 1);
      *pcurX += FONTW;
      if (*pcurX >= X0 + CONSOLE_NX * FONTW)
        cons_newline(cons, pcurX, pcurY);
      if (((*pcurX - X0) & 0x1f) == 0)
        break;
    }
    break;
  case 0x0a:  // Line feed.
    cons_newline(cons, pcurX, pcurY);
    break;
  case 0x0d:  // Carrige return.
    *pcurX = 8;  // TODO: Consider multiple lines.
    break;
  default:  // Normal character.
    if (cons->sheet != NULL)
      putfonts8_asc_sht(cons->shtctl, cons->sheet, *pcurX, *pcurY, fontColor, backColor, s, 1);
    if (move) {
      *pcurX += FONTW;
      if (*pcurX >= X0 + CONSOLE_NX * FONTW)
        cons_newline(cons, pcurX, pcurY);
    }
    break;
  }
}

void cons_putchar(CONSOLE* cons, int chr, char move, char neg) {
  cons_putchar_at(cons, chr, move, neg, &cons->cur_x, &cons->cur_y);
}

void cons_putstr0(CONSOLE* cons, const char* s) {
  for (; *s != '\0'; ++s)
    cons_putchar(cons, *s, TRUE, FALSE);
}

void cons_putstr1(CONSOLE* cons, const char* s, int l) {
  for (int i = 0; i < l; ++i)
    cons_putchar(cons, *s++, TRUE, FALSE);
}

static void cons_runcmd(const char* cmdline, CONSOLE* cons) {
  if (strcmp(cmdline, "mem") == 0 && cons->sheet != NULL) {
    cmd_mem(cons);
  } else if (strcmp(cmdline, "dir") == 0 && cons->sheet != NULL) {
    cmd_dir(cons);
  } else if (strcmp(cmdline, "exit") == 0) {
    cmd_exit(cons);
  } else if (strncmp(cmdline, "start ", 6) == 0) {
    cmd_start(cmdline);
  } else if (strncmp(cmdline, "ncst ", 5) == 0) {
    cmd_ncst(cmdline);
  } else if (strcmp(cmdline, "fat") == 0) {
    cmd_fat(cons);
  } else if (strcmp(cmdline, "dir2") == 0) {
    cmd_dir2(cons);
  } else if (cmdline[0] != '\0') {
    if (!cmd_app(cons, cmdline))
      cons_putstr0(cons, "Bad command.\n");
  }
}

static void cursor_left(CONSOLE* cons) {
  --cons->cmdp;
  cons->cur_x -= FONTW;
  if (cons->cur_x < 8) {
    cons->cur_x = 8 + (CONSOLE_NX - 1) * FONTW;
    cons->cur_y -= FONTH;
  }
}

static void cursor_right(CONSOLE* cons) {
  ++cons->cmdp;
  cons->cur_x += FONTW;
  if (cons->cur_x >= 8 + CONSOLE_NX * FONTW) {
    cons->cur_x = 8;
    cons->cur_y += FONTH;
  }
}

static void draw_cmdline(CONSOLE* cons, const char* cmdline) {
  int curx = cons->cur_x, cury = cons->cur_y;
  for (int i = cons->cmdp, n = cons->cmdlen; i < n; ++i)
    cons_putchar_at(cons, cmdline[i], TRUE, FALSE, &curx, &cury);
}

static void putPrompt(CONSOLE* cons) {
  unsigned char col = cons->fontColor;
  cons->fontColor = COL8_BLUE;
  cons_putchar(cons, '>', TRUE, FALSE);
  cons->fontColor = col;
}

static void handle_key_event(CONSOLE* cons, char* cmdline, unsigned char key) {
  cons->cur_c = COL8_WHITE;
  switch (key) {
  case 10:  // Enter.
  case 0x0d:  // CTRL-M
    // Erase cursor and newline.
    cons_putchar(cons, cmdline[cons->cmdp], FALSE, FALSE);
    cmdline[cons->cmdlen] = '\0';
    cons_newline(cons, &cons->cur_x, &cons->cur_y);
    cons_runcmd(cmdline, cons);
    if (cons->sheet == NULL)
      cmd_exit(cons);
    putPrompt(cons);
    cons->cmdp = cons->cmdlen = 0;
    cmdline[0] = ' ';
    break;
  case 8:  // Back space.
    if (cons->cmdp > 0) {
      cons_putchar(cons, cmdline[cons->cmdp], FALSE, FALSE);  // Erase cursor.
      if (cons->cur_x > 8)
        cons->cur_x -= FONTW;
      else {
        cons->cur_x = (CONSOLE_NX - 1) * FONTW + X0;
        cons->cur_y -= FONTH;
      }
      memmove(&cmdline[cons->cmdp - 1], &cmdline[cons->cmdp], cons->cmdlen - cons->cmdp + 1);
      --cons->cmdp;
      draw_cmdline(cons, cmdline);
      --cons->cmdlen;
    }
    break;
  case 0x04:  // CTRL+D : Delete
    if (cons->cmdp < cons->cmdlen) {
      memmove(&cmdline[cons->cmdp], &cmdline[cons->cmdp + 1], cons->cmdlen - cons->cmdp + 1);
      draw_cmdline(cons, cmdline);
      --cons->cmdlen;
    }
    break;
  case KEY_LEFT:
  case 0x02:  // CTRL+B
    if (cons->cmdp > 0) {
      cons_putchar(cons, cmdline[cons->cmdp], FALSE, FALSE);
      cursor_left(cons);
    }
    break;
  case KEY_RIGHT:
  case 0x06:  // CTRL+F
    if (cons->cmdp < cons->cmdlen) {
      cons_putchar(cons, cmdline[cons->cmdp], FALSE, FALSE);
      cursor_right(cons);
    }
    break;
  case 0x01:  // CTRL-A : Line top.
    if (cons->cmdp > 0) {
      cons_putchar(cons, cmdline[cons->cmdp], FALSE, FALSE);
      while (cons->cmdp > 0)
        cursor_left(cons);
    }
    break;
  case 0x05:  // CTRL-E : Line last.
    if (cons->cmdp < cons->cmdlen) {
      cons_putchar(cons, cmdline[cons->cmdp], FALSE, FALSE);
      while (cons->cmdp < cons->cmdlen)
        cursor_right(cons);
    }
    break;
  case 0x0b:  // CTRL-K : Erase after cursor.
    if (cons->cmdlen > cons->cmdp) {
      // Erasee.
      int curx = cons->cur_x, cury = cons->cur_y;
      for (int i = cons->cmdp, n = cons->cmdlen; i < n; ++i)
        cons_putchar_at(cons, ' ', TRUE, FALSE, &curx, &cury);

      cons->cmdlen = cons->cmdp;
      cmdline[cons->cmdlen] = ' ';
    }
    break;
  case 0x0c:  // CTRL-L : Erase screen.
    cons_cls(cons);
    putPrompt(cons);
    {
      int cmdp = cons->cmdp;
      cons->cmdp = 0;
      draw_cmdline(cons, cmdline);
      for (int i = 0; i < cmdp; ++i)
        cursor_right(cons);
    }
    break;
  default:  // Normal character.
    if (' ' <= key && key < 0x80) {
      if (cons->cmdlen < CMDLINE_MAX) {
        ++cons->cmdlen;
        memmove(&cmdline[cons->cmdp + 1], &cmdline[cons->cmdp], cons->cmdlen - cons->cmdp);
        cmdline[cons->cmdp] = key;
        draw_cmdline(cons, cmdline);
        cursor_right(cons);
      }
    }
    break;
  }
}

static void console_task(SHTCTL* shtctl, SHEET* sheet) {
  TASK* task = task_now();

#define TASK_FHANDLE_COUNT  (8)
  FDHANDLE fhandle[TASK_FHANDLE_COUNT];
  for (int i = 0; i < TASK_FHANDLE_COUNT; ++i)
    fhandle[i].finfo = NULL;  // Not used.
  task->fhandle = fhandle;
  task->fhandleCount = TASK_FHANDLE_COUNT;

  char cmdline[CMDLINE_MAX + 1];

  // Show prompt.
  CONSOLE cons;
  cons.shtctl = shtctl;
  cons.sheet = sheet;
  cons.cur_x = X0;
  cons.cur_y = Y0;
  cons.cur_c = -1;
  cons.cmdp = cons.cmdlen = 0;
  cons.fontColor = COL8_WHITE;
  cons.bgColor = COL8_BLACK;
  task->cons = &cons;
  task->cmdline = cmdline;
  cmdline[0] = ' ';

  if (cons.sheet != NULL) {
    cons.timer = timer_alloc();
    timer_init(cons.timer, &task->fifo, 1);
    timer_settime(cons.timer, 50);
    cons_cls(&cons);
  }

  putPrompt(&cons);

  for (;;) {
    io_cli();
    if (fifo_empty(&task->fifo)) {
      task_sleep(task);
      continue;
    }
    int i = fifo_get(&task->fifo);
    io_sti();
    if (256 <= i && i < 512) {  // Keyboard data (from task A).
      handle_key_event(&cons, cmdline, i - 256);
    } else {
      switch (i) {
      case 0:
      case 1:
        if (cons.sheet != NULL) {
          if (cons.cur_c >= 0)
            cons.cur_c = i == 0 ? COL8_WHITE : COL8_BLACK;
          timer_init(cons.timer, &task->fifo, 1 - i);
          timer_settime(cons.timer, 50);
        }
        break;
      case 2:  cons.cur_c = COL8_WHITE; break;
      case 3:
        if (cons.sheet != NULL) {
          cons_putchar(&cons, cmdline[cons.cmdp], FALSE, FALSE);
          boxfill8(cons.sheet->buf, cons.sheet->bxsize, COL8_WHITE, cons.cur_x, cons.cur_y + FONTH - 1, cons.cur_x + FONTW, cons.cur_y + FONTH);
        }
        cons.cur_c = -1;
        break;
      case 4:  // Close button clicked.
        cmd_exit(&cons);
        break;
      }
    }
    // Redraw cursor.
    if (cons.sheet != NULL) {
      if (cons.cur_c >= 0)
        cons_putchar(&cons, cmdline[cons.cmdp], FALSE, cons.cur_c == COL8_WHITE);
      else
        sheet_refresh(shtctl, cons.sheet, cons.cur_x, cons.cur_y, cons.cur_x + 8, cons.cur_y + 16);
    }
  }
}

TASK* open_constask(SHTCTL* shtctl, SHEET* sheet) {
  MEMMAN *memman = (MEMMAN*)MEMMAN_ADDR;
  TASK* task = task_alloc();
  int stack_size = 64 * 1024;
  task->cons_stack = memman_alloc_4k(memman, stack_size);
  task->tss.esp = (int)task->cons_stack + stack_size - 4 - 4 * 2;
  task->tss.eip = (int) &console_task;
  task->tss.cs = 2 * 8;
  task->tss.es = task->tss.ss = task->tss.ds = task->tss.fs = task->tss.gs = 1 * 8;
  *((int*)(task->tss.esp + 4)) = (int)shtctl;
  *((int*)(task->tss.esp + 8)) = (int)sheet;
  task_run(task, 2, 2);

  int* cons_fifo = (int*)memman_alloc_4k(memman, 128 * sizeof(int));
  fifo_init(&task->fifo, 128, cons_fifo, task);
  return task;
}

SHEET* open_console(SHTCTL* shtctl) {
  MEMMAN *memman = (MEMMAN*)MEMMAN_ADDR;
  SHEET* sheet = sheet_alloc(shtctl);
  unsigned char* buf = (unsigned char*)memman_alloc_4k(memman, CONSOLE_WIDTH * CONSOLE_HEIGHT);
  sheet_setbuf(sheet, buf, CONSOLE_WIDTH, CONSOLE_HEIGHT, -1);
  make_window8(sheet, "console", FALSE);
  make_textbox8(sheet, X0, Y0, CONSOLE_NX * FONTW, CONSOLE_NY * FONTH, COL8_BLACK);
  sheet->task = open_constask(shtctl, sheet);
  sheet->flags |= 0x20;
  return sheet;
}

void close_constask(TASK* task) {
  MEMMAN *memman = (MEMMAN*)MEMMAN_ADDR;
  task_sleep(task);
  if (task->cons_stack != NULL)
    memman_free_4k(memman, task->cons_stack, 64 * 1024);
  memman_free_4k(memman, task->fifo.buf, 128 * sizeof(int));
  io_cli();
  if (taskctl->task_fpu == task)
    taskctl->task_fpu = NULL;
  io_sti();
  task_free(task);
}

void close_console(SHTCTL* shtctl, SHEET* sheet) {
  MEMMAN *memman = (MEMMAN*)MEMMAN_ADDR;
  close_constask(sheet->task);
  memman_free_4k(memman, sheet->buf, CONSOLE_WIDTH * CONSOLE_HEIGHT);  // Warn! sheet size.
  sheet_free(shtctl, sheet);
}
