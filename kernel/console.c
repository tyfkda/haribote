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

// .hrb executable file header.
typedef struct {
  uint32_t segSize;
  int8_t signature[4];  // Must be "Hari"
  uint32_t mmarea;
  uint32_t esp;  // stackSize;
  uint32_t dataSize;
  uint32_t dataAdr;
  int32_t jump;
  uint32_t entryPoint;
  uint32_t heapAdr;
  int32_t dummy[3];
} HrbHeader;

static TASK* open_constask(SHTCTL* shtctl, SHEET* sheet, unsigned int memtotal);

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
  boxfill8(buf, bxsize, COL8_BLACK, X0, Y0 + (CONSOLE_NY - 1) * FONTH, X0 + CONSOLE_NX * FONTW, Y0 + CONSOLE_NY * FONTH);
  sheet_refresh(cons->shtctl, sheet, X0, Y0, X0 + CONSOLE_NX * FONTW, Y0 + CONSOLE_NY * FONTH);
}

void cons_putchar_with(CONSOLE* cons, int chr, char move, char neg, int* pcurX, int* pcurY) {
  unsigned char fontColor = COL8_WHITE, backColor = COL8_BLACK;
  if (neg)
    fontColor = COL8_BLACK, backColor = COL8_WHITE;
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
  cons_putchar_with(cons, chr, move, neg, &cons->cur_x, &cons->cur_y);
}

void cons_putstr0(CONSOLE* cons, const char* s) {
  for (; *s != '\0'; ++s)
    cons_putchar(cons, *s, TRUE, FALSE);
}

void cons_putstr1(CONSOLE* cons, const char* s, int l) {
  for (int i = 0; i < l; ++i)
    cons_putchar(cons, *s++, TRUE, FALSE);
}

static void cmd_mem(CONSOLE* cons, int memtotal) {
  MEMMAN* memman = (MEMMAN*)MEMMAN_ADDR;
  char s[60];
  sprintf(s, "total %4dMB\nfree %5dKB\n",
          memtotal / (1024 * 1024),
          memman_total(memman) / 1024);
  cons_putstr0(cons, s);
}

static void cmd_cls(CONSOLE* cons) {
  SHEET* sheet = cons->sheet;
  boxfill8(sheet->buf, sheet->bxsize, COL8_BLACK, 8, 28, 8 + CONSOLE_NX * 8, 28 + CONSOLE_NY * 16);
  sheet_refresh(cons->shtctl, sheet, 8, 28, 8 + CONSOLE_NX * 8, 28 + CONSOLE_NY * 16);
  cons->cur_y = 28;
}

static void cmd_dir(CONSOLE* cons) {
  FDINFO *finfo = (FDINFO*)(ADR_DISKIMG + 0x002600);
  for (int i = 0; i < 224; ++i) {
    FDINFO* p = &finfo[i];
    if (p->name[0] == 0x00)  // End of table.
      break;
    if (p->name[0] == 0xe5)  // Deleted file.
      continue;
    if ((p->type & 0x18) == 0) {
      char s[30];
      sprintf(s, "filename.ext   %7d\n", p->size);
      memcpy(&s[0], p->name, 8);
      memcpy(&s[9], p->ext, 3);
      if (p->ext[0] == ' ')  // No file extension: remove dot.
        s[8] = ' ';
      cons_putstr0(cons, s);
    }
  }
}

static void cmd_exit(CONSOLE* cons) {
  TASK* task = task_now();
  SHTCTL* shtctl = getOsInfo()->shtctl;
  FIFO* fifo = getOsInfo()->fifo;
  if (cons->sheet != NULL)
    timer_cancel(cons->timer);
  io_cli();
  if (cons->sheet != NULL)
    fifo_put(fifo, cons->sheet - shtctl->sheets0 + 768);  // 768~1023
  else
    fifo_put(fifo, cons->sheet - shtctl->sheets0 + 1024);  // 1024~2023
  io_sti();
  for (;;)
    task_sleep(task);
}

static void cmd_start(const char* cmdline, int memtotal) {
  SHTCTL* shtctl = getOsInfo()->shtctl;
  SHEET* sheet = open_console(shtctl, memtotal);
  sheet_slide(shtctl, sheet, 32, 4);
  sheet_updown(shtctl, sheet, shtctl->top);

  // Send key command.
  FIFO* fifo = &sheet->task->fifo;
  for (int i = 6; cmdline[i] != 0; ++i)
    fifo_put(fifo, cmdline[i] + 256);
  fifo_put(fifo, 10 + 256);  // Enter.
}

// No console start.
static void cmd_ncst(const char* cmdline, int memtotal) {
  TASK* task = open_constask(NULL, NULL, memtotal);

  // Send key command.
  FIFO* fifo = &task->fifo;
  for (int i = 5; cmdline[i] != 0; ++i)
    fifo_put(fifo, cmdline[i] + 256);
  fifo_put(fifo, 10 + 256);  // Enter.
}

static char cmd_app(CONSOLE* cons, const char* cmdline) {
  char name[13];
  int i;
  for (i = 0; i < 8; ++i) {
    if (cmdline[i] <= ' ')
      break;
    name[i] = cmdline[i];
  }
  name[i] = '\0';

  FDHANDLE fh;
  if (!fd_open(&fh, name)) {
    // Try executable extension.
    strcpy(name + strlen(name), ".hrb");
    if (!fd_open(&fh, name))
      return FALSE;
  }

  // File found.
  HrbHeader header;
  int readSize = fd_read(&fh, &header, sizeof(header));
  if ((size_t)readSize < sizeof(header) || strncmp((char*)header.signature, "Hari", 4) != 0) {
    cons_putstr0(cons, ".hrb file format error.\n");
    return FALSE;
  }

  int codeBlockSize = fh.finfo->size - header.dataSize;

  MEMMAN *memman = (MEMMAN*) MEMMAN_ADDR;
  char* code = (char*)memman_alloc_4k(memman, codeBlockSize);
  char* data = (char*)memman_alloc_4k(memman, header.segSize);  // Data segment.
  memcpy(code, &header, sizeof(header));
  int codeReadSize = fd_read(&fh, code + sizeof(header), codeBlockSize - sizeof(header));
  int dataReadSize = fd_read(&fh, data + header.esp, header.dataSize);
  if (codeReadSize != codeBlockSize - (int)sizeof(header) ||
      dataReadSize != (int)header.dataSize) {
    cons_putstr0(cons, "File size mismatch.\n");
    memman_free_4k(memman, data, header.segSize);
    memman_free_4k(memman, code, codeBlockSize);
    return FALSE;
  }

  // Clear .bss area.
  memset(data + header.esp + header.dataSize, 0x00, header.segSize - (header.esp + header.dataSize));

  TASK* task = task_now();
  task->ds_base = (int)data;  // Store data segment address.

  set_segmdesc(task->ldt + 0, codeBlockSize - 1, (int)code, AR_CODE32_ER + 0x60);
  set_segmdesc(task->ldt + 1, header.segSize - 1, (int)data, AR_DATA32_RW + 0x60);
  start_app(0x1b, 0 * 8 + 4, header.esp, 1 * 8 + 4, &(task->tss.esp0));

  // End of application.
  // Free sheets which are opened by the task.
  SHTCTL* shtctl = getOsInfo()->shtctl;
  for (int i = 0; i < MAX_SHEETS; ++i) {
    SHEET* sheet = &shtctl->sheets0[i];
    if ((sheet->flags & 0x11) == 0x11 && sheet->task == task)
      sheet_free(shtctl, sheet);
  }
  // Close files.
  for (int i = 0; i < task->fhandleCount; ++i)
    fd_close(&task->fhandle[i]);
  timer_cancelall(&task->fifo);
  memman_free_4k(memman, data, header.segSize);
  memman_free_4k(memman, code, codeBlockSize);
  return TRUE;
}

int* inthandler0c(int* esp) {
  // esp[ 0] = edi  : esp[0~7] are given from asm_inthandler, pushal
  // esp[ 1] = esi
  // esp[ 2] = ebp
  // esp[ 4] = ebx
  // esp[ 5] = edx
  // esp[ 6] = ecx
  // esp[ 7] = eax
  // esp[ 8] = ds   : esp[8~9] are given from asm_inthandler, push
  // esp[ 9] = es
  // esp[10] = error code (0)
  // esp[11] = eip
  // esp[12] = cs
  // esp[13] = eflags
  // esp[14] = esp  : esp for application
  // esp[15] = ss   : ss for application
  TASK* task = task_now();
  CONSOLE* cons = task->cons;
  cons_putstr0(cons, "\nINT 0C :\n Stack Exception.\n");
  char s[30];
  sprintf(s, "EIP = %08x\n", esp[11]);
  cons_putstr0(cons, s);
  return &task->tss.esp0;  // Abort
}

int* inthandler0d(void) {
  TASK* task = task_now();
  CONSOLE* cons = task->cons;
  cons_putstr0(cons, "\nINT 0D :\n General Protected Exception.\n");
  return &task->tss.esp0;  // Abort
}

static void cons_runcmd(const char* cmdline, CONSOLE* cons, int memtotal) {
  if (strcmp(cmdline, "mem") == 0 && cons->sheet != NULL) {
    cmd_mem(cons, memtotal);
  } else if (strcmp(cmdline, "cls") == 0 && cons->sheet != NULL) {
    cmd_cls(cons);
  } else if (strcmp(cmdline, "dir") == 0 && cons->sheet != NULL) {
    cmd_dir(cons);
  } else if (strcmp(cmdline, "exit") == 0) {
    cmd_exit(cons);
  } else if (strncmp(cmdline, "start ", 6) == 0) {
    cmd_start(cmdline, memtotal);
  } else if (strncmp(cmdline, "ncst ", 5) == 0) {
    cmd_ncst(cmdline, memtotal);
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
    cons_putchar_with(cons, cmdline[i], TRUE, FALSE, &curx, &cury);
}

static void handle_key_event(CONSOLE* cons, char* cmdline, unsigned int memtotal, unsigned char key) {
  cons->cur_c = COL8_WHITE;
  switch (key) {
  case 10:  // Enter.
  case 0x0d:  // CTRL-M
    // Erase cursor and newline.
    cons_putchar(cons, cmdline[cons->cmdp], FALSE, FALSE);
    cmdline[cons->cmdlen] = '\0';
    cons_newline(cons, &cons->cur_x, &cons->cur_y);
    cons_runcmd(cmdline, cons, memtotal);
    if (cons->sheet == NULL)
      cmd_exit(cons);
    cons_putchar(cons, '>', TRUE, FALSE);
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
        cons_putchar_with(cons, ' ', TRUE, FALSE, &curx, &cury);

      cons->cmdlen = cons->cmdp;
      cmdline[cons->cmdlen] = ' ';
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

static void console_task(SHTCTL* shtctl, SHEET* sheet, unsigned int memtotal) {
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
  task->cons = &cons;
  task->cmdline = cmdline;
  cmdline[0] = ' ';

  if (cons.sheet != NULL) {
    cons.timer = timer_alloc();
    timer_init(cons.timer, &task->fifo, 1);
    timer_settime(cons.timer, 50);
  }

  cons_putchar(&cons, '>', TRUE, FALSE);

  for (;;) {
    io_cli();
    if (fifo_empty(&task->fifo)) {
      task_sleep(task);
      continue;
    }
    int i = fifo_get(&task->fifo);
    io_sti();
    if (256 <= i && i < 512) {  // Keyboard data (from task A).
      handle_key_event(&cons, cmdline, memtotal, i - 256);
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

static TASK* open_constask(SHTCTL* shtctl, SHEET* sheet, unsigned int memtotal) {
  MEMMAN *memman = (MEMMAN*)MEMMAN_ADDR;
  TASK* task = task_alloc();
  int stack_size = 64 * 1024;
  task->cons_stack = memman_alloc_4k(memman, stack_size);
  task->tss.esp = (int)task->cons_stack + stack_size - 4 - 4 * 3;
  task->tss.eip = (int) &console_task;
  task->tss.cs = 2 * 8;
  task->tss.es = task->tss.ss = task->tss.ds = task->tss.fs = task->tss.gs = 1 * 8;
  *((int*)(task->tss.esp + 4)) = (int)shtctl;
  *((int*)(task->tss.esp + 8)) = (int)sheet;
  *((int*)(task->tss.esp + 12)) = (int)memtotal;
  task_run(task, 2, 2);

  int* cons_fifo = (int*)memman_alloc_4k(memman, 128 * sizeof(int));
  fifo_init(&task->fifo, 128, cons_fifo, task);
  return task;
}

SHEET* open_console(SHTCTL* shtctl, unsigned int memtotal) {
  MEMMAN *memman = (MEMMAN*)MEMMAN_ADDR;
  SHEET* sheet = sheet_alloc(shtctl);
  unsigned char* buf = (unsigned char*)memman_alloc_4k(memman, CONSOLE_WIDTH * CONSOLE_HEIGHT);
  sheet_setbuf(sheet, buf, CONSOLE_WIDTH, CONSOLE_HEIGHT, -1);
  make_window8(sheet, "console", FALSE);
  make_textbox8(sheet, X0, Y0, CONSOLE_NX * 8, CONSOLE_NY * 16, COL8_BLACK);
  sheet->task = open_constask(shtctl, sheet, memtotal);
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
