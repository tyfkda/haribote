#include "console.h"
#include "bootpack.h"
#include "file.h"
#include "graphics.h"
#include "mtask.h"
#include "sheet.h"
#include "stdio.h"
#include "string.h"
#include "timer.h"
#include "window.h"

static TASK* open_constask(SHTCTL* shtctl, SHEET* sht, unsigned int memtotal);

void cons_putchar(CONSOLE* cons, int chr, char move) {
  char s[2] = { chr, '\0' };
  switch (chr) {
  case 0x09:  // Tab.
    for (;;) {
      if (cons->sheet != NULL)
        putfonts8_asc_sht(cons->shtctl, cons->sheet, cons->cur_x, cons->cur_y, COL8_WHITE, COL8_BLACK, " ", 1);
      cons->cur_x += 8;
      if (cons->cur_x >= 8 + 240)
        cons_newline(cons);
      if (((cons->cur_x - 8) & 0x1f) == 0)
        break;
    }
    break;
  case 0x0a:  // Line feed.
    cons_newline(cons);
    break;
  case 0x0d:  // Carrige return.
    break;
  default:  // Normal character.
    if (cons->sheet != NULL)
      putfonts8_asc_sht(cons->shtctl, cons->sheet, cons->cur_x, cons->cur_y, COL8_WHITE, COL8_BLACK, s, 1);
    if (move) {
      cons->cur_x += 8;
      if (cons->cur_x == 8 + 240)
        cons_newline(cons);
    }
    break;
  }
}

void cons_putstr0(CONSOLE* cons, char* s) {
  for (; *s != '\0'; ++s)
    cons_putchar(cons, *s, 1);
}

void cons_putstr1(CONSOLE* cons, char* s, int l) {
  for (int i = 0; i < l; ++i)
    cons_putchar(cons, *s++, 1);
}

void cons_newline(CONSOLE* cons) {
  cons->cur_x = 8;
  SHEET* sheet = cons->sheet;
  if (cons->cur_y < 28 + 112 || sheet == NULL) {
    cons->cur_y += 16;
    return;
  }
  unsigned char* buf = sheet->buf;
  int bxsize = sheet->bxsize;
  // Scroll.
  for (int y = 28; y < 28 + 112; ++y)
    memcpy(&buf[y * bxsize + 8], &buf[(y + 16) * bxsize + 8], 240);
  // Erase last line.
  boxfill8(buf, bxsize, COL8_BLACK, 8, 28 + 112, 8 + 240, 28 + 112 + 16);
  sheet_refresh(cons->shtctl, sheet, 8, 28, 8 + 240, 28 + 128);
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
  boxfill8(sheet->buf, sheet->bxsize, COL8_BLACK, 8, 28, 8 + 240, 28 + 128);
  sheet_refresh(cons->shtctl, sheet, 8, 28, 8 + 240, 28 + 128);
  cons->cur_y = 28;
}

static void cmd_dir(CONSOLE* cons) {
  FILEINFO *finfo = (FILEINFO*)(ADR_DISKIMG + 0x002600);
  for (int i = 0; i < 224; ++i) {
    FILEINFO* p = &finfo[i];
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

static void cmd_exit(CONSOLE* cons, const short* fat) {
  MEMMAN *memman = (MEMMAN*) MEMMAN_ADDR;
  TASK* task = task_now();
  SHTCTL* shtctl = (SHTCTL*)*((int*)0x0fe4);
  FIFO* fifo = (FIFO*)*((int*)0x0fec);
  if (cons->sheet != NULL)
    timer_cancel(cons->timer);
  memman_free_4k(memman, (void*)fat, 4 * 2880);
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
  SHTCTL* shtctl = (SHTCTL*)*((int*)0x0fe4);
  SHEET* sht = open_console(shtctl, memtotal);
  sheet_slide(shtctl, sht, 32, 4);
  sheet_updown(shtctl, sht, shtctl->top);

  // Send key command.
  FIFO* fifo = &sht->task->fifo;
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

static char cmd_app(CONSOLE* cons, const short* fat, const char* cmdline) {
  char name[13];
  int i;
  for (i = 0; i < 8; ++i) {
    if (cmdline[i] <= ' ')
      break;
    name[i] = cmdline[i];
  }
  name[i] = '\0';

  FILEINFO *finfo = file_search(name, (FILEINFO*)(ADR_DISKIMG + 0x002600), 224);
  if (finfo == NULL) {
    // Try executable extension.
    strcpy(name + strlen(name), ".hrb");
    finfo = file_search(name, (FILEINFO*)(ADR_DISKIMG + 0x002600), 224);
    if (finfo == NULL)
      return FALSE;
  }

  // File found.
  MEMMAN *memman = (MEMMAN*) MEMMAN_ADDR;
  char* p = (char*)memman_alloc_4k(memman, finfo->size);
  file_loadfile(finfo, fat, (char*)(ADR_DISKIMG + 0x003e00), p);
  if (finfo->size < 36 || strncmp(p + 4, "Hari", 4) != 0 || *p != 0x00) {
    cons_putstr0(cons, ".hrb file format error.\n");
  } else {
    int segsiz = *((int*)(p + 0x0000));
    int esp    = *((int*)(p + 0x000c));
    int datsiz = *((int*)(p + 0x0010));
    int dathrb = *((int*)(p + 0x0014));
    char* q = (char*)memman_alloc_4k(memman, segsiz);  // Data segment.
    TASK* task = task_now();
    task->ds_base = (int)q;  // Store data segment address.

    set_segmdesc(task->ldt + 0, finfo->size - 1, (int)p, AR_CODE32_ER + 0x60);
    set_segmdesc(task->ldt + 1, segsiz - 1, (int)q, AR_DATA32_RW + 0x60);
    memcpy(&q[esp], &p[dathrb], datsiz);
    start_app(0x1b, 0 * 8 + 4, esp, 1 * 8 + 4, &(task->tss.esp0));

    // End of application.
    // Free sheets which are opened by the task.
    SHTCTL* shtctl = (SHTCTL*)*((int*)0x0fe4);
    for (int i = 0; i < MAX_SHEETS; ++i) {
      SHEET* sht = &shtctl->sheets0[i];
      if ((sht->flags & 0x11) == 0x11 && sht->task == task)
        sheet_free(shtctl, sht);
    }
    // Close files.
    for (int i = 0; i < 8; ++i) {
      if (task->fhandle[i].finfo != NULL) {
        //memman_free_4k(memman, task->fhandle[i].buf, task->fhandle[i].size);
        task->fhandle[i].finfo = NULL;
      }
    }
    timer_cancelall(&task->fifo);
    memman_free_4k(memman, q, 64 * 1024);
  }
  memman_free_4k(memman, p, finfo->size);
  cons_newline(cons);
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

static void cons_runcmd(const char* cmdline, CONSOLE* cons, const short* fat, int memtotal) {
  if (strcmp(cmdline, "mem") == 0 && cons->sheet != NULL) {
    cmd_mem(cons, memtotal);
  } else if (strcmp(cmdline, "cls") == 0 && cons->sheet != NULL) {
    cmd_cls(cons);
  } else if (strcmp(cmdline, "dir") == 0 && cons->sheet != NULL) {
    cmd_dir(cons);
  } else if (strcmp(cmdline, "exit") == 0) {
    cmd_exit(cons, fat);
  } else if (strncmp(cmdline, "start ", 6) == 0) {
    cmd_start(cmdline, memtotal);
  } else if (strncmp(cmdline, "ncst ", 5) == 0) {
    cmd_ncst(cmdline, memtotal);
  } else if (cmdline[0] != '\0') {
    if (!cmd_app(cons, fat, cmdline))
      cons_putstr0(cons, "Bad command.\n");
  }
}

static void handle_key_event(CONSOLE* cons, char* cmdline, const short* fat, unsigned int memtotal, unsigned char key) {
  switch (key) {
  case 10:  // Enter.
    // Erase cursor and newline.
    cons_putchar(cons, ' ', FALSE);
    cmdline[cons->cur_x / 8 - 2] = '\0';
    cons_newline(cons);
    cons_runcmd(cmdline, cons, fat, memtotal);
    if (cons->sheet == NULL)
      cmd_exit(cons, fat);
    cons_putchar(cons, '>', TRUE);
    break;
  case 8:  // Back space.
    if (cons->cur_x > 16) {
      cons_putchar(cons, ' ', FALSE);
      cons->cur_x -= 8;
    }
    break;
  default:  // Normal character.
    if (' ' <= key && key < 0x80) {
      if (cons->cur_x < 240) {
        cmdline[cons->cur_x / 8 - 2] = key;
        cons_putchar(cons, key, TRUE);
      }
    }
    break;
  }
}

static void console_task(SHTCTL* shtctl, SHEET* sheet, unsigned int memtotal) {
  TASK* task = task_now();

  MEMMAN *memman = (MEMMAN*) MEMMAN_ADDR;
  short* fat = (short*)memman_alloc_4k(memman, sizeof(short) * 2880);
  file_readfat(fat, (unsigned char*)(ADR_DISKIMG + 0x000200));

  FILEHANDLE fhandle[8];
  for (int i = 0; i < 8; ++i)
    fhandle[i].finfo = NULL;  // Not used.
  task->fhandle = fhandle;
  task->fat = fat;

  char cmdline[30];

  // Show prompt.
  CONSOLE cons;
  cons.shtctl = shtctl;
  cons.sheet = sheet;
  cons.cur_x = 8;
  cons.cur_y = 28;
  cons.cur_c = -1;
  task->cons = &cons;
  task->cmdline = cmdline;

  if (cons.sheet != NULL) {
    cons.timer = timer_alloc();
    timer_init(cons.timer, &task->fifo, 1);
    timer_settime(cons.timer, 50);
  }

  cons_putchar(&cons, '>', TRUE);

  for (;;) {
    io_cli();
    if (fifo_empty(&task->fifo)) {
      task_sleep(task);
      continue;
    }
    int i = fifo_get(&task->fifo);
    io_sti();
    if (256 <= i && i < 512) {  // Keyboard data (from task A).
      handle_key_event(&cons, cmdline, fat, memtotal, i - 256);
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
        if (cons.sheet != NULL)
          boxfill8(sheet->buf, sheet->bxsize, COL8_BLACK, cons.cur_x, cons.cur_y, cons.cur_x + 8, cons.cur_y + 16);
        cons.cur_c = -1;
        break;
      case 4:  // Close button clicked.
        cmd_exit(&cons, fat);
        break;
      }
    }
    // Redraw cursor.
    if (cons.sheet != NULL) {
      if (cons.cur_c >= 0)
        boxfill8(cons.sheet->buf, cons.sheet->bxsize, cons.cur_c, cons.cur_x, cons.cur_y, cons.cur_x + 8, cons.cur_y + 16);
      sheet_refresh(shtctl, cons.sheet, cons.cur_x, cons.cur_y, cons.cur_x + 8, cons.cur_y + 16);
    }
  }
}

static TASK* open_constask(SHTCTL* shtctl, SHEET* sht, unsigned int memtotal) {
  MEMMAN *memman = (MEMMAN*)MEMMAN_ADDR;
  TASK* task = task_alloc();
  int stack_size = 64 * 1024;
  task->cons_stack = memman_alloc_4k(memman, stack_size);
  task->tss.esp = (int)task->cons_stack + stack_size - 4 - 4 * 3;
  task->tss.eip = (int) &console_task;
  task->tss.cs = 2 * 8;
  task->tss.es = task->tss.ss = task->tss.ds = task->tss.fs = task->tss.gs = 1 * 8;
  *((int*)(task->tss.esp + 4)) = (int)shtctl;
  *((int*)(task->tss.esp + 8)) = (int)sht;
  *((int*)(task->tss.esp + 12)) = (int)memtotal;
  task_run(task, 2, 2);

  int* cons_fifo = (int*)memman_alloc_4k(memman, 128 * sizeof(int));
  fifo_init(&task->fifo, 128, cons_fifo, task);
  return task;
}

SHEET* open_console(SHTCTL* shtctl, unsigned int memtotal) {
  MEMMAN *memman = (MEMMAN*)MEMMAN_ADDR;
  SHEET* sht = sheet_alloc(shtctl);
  unsigned char* buf = (unsigned char*)memman_alloc_4k(memman, 256 * 165);
  sheet_setbuf(sht, buf, 256, 165, -1);
  make_window8(buf, 256, 165, "console", FALSE);
  make_textbox8(sht, 8, 28, 240, 128, COL8_BLACK);
  sht->task = open_constask(shtctl, sht, memtotal);
  sht->flags |= 0x20;
  return sht;
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

void close_console(SHTCTL* shtctl, SHEET* sht) {
  MEMMAN *memman = (MEMMAN*)MEMMAN_ADDR;
  close_constask(sht->task);
  memman_free_4k(memman, sht->buf, 256 * 165);  // Warn! sheet size.
  sheet_free(shtctl, sht);
}
