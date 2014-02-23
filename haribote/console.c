#include "console.h"
#include "bootpack.h"
#include "dsctbl.h"
#include "fifo.h"
#include "file.h"
#include "graphics.h"
#include "mtask.h"
#include "sheet.h"
#include "stdio.h"
#include "string.h"
#include "timer.h"
#include "window.h"

#define API_PUTCHAR  (1)
#define API_PUTSTR0  (2)
#define API_PUTSTR1  (3)
#define API_END  (4)
#define API_OPENWIN  (5)
#define API_PUTSTRWIN  (6)
#define API_BOXFILWIN  (7)
#define API_INITMALLOC  (8)
#define API_MALLOC  (9)
#define API_FREE  (10)
#define API_POINT  (11)
#define API_REFRESHWIN  (12)
#define API_LINEWIN  (13)
#define API_CLOSEWIN  (14)
#define API_GETKEY  (15)
#define API_ALLOCTIMER  (16)
#define API_INITTIMER  (17)
#define API_SETTIMER  (18)
#define API_FREETIMER  (19)
#define API_BEEP  (20)
#define API_FOPEN  (21)
#define API_FCLOSE  (22)
#define API_FSEEK  (23)
#define API_FSIZE  (24)
#define API_FREAD  (25)
#define API_CMDLINE  (26)

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

int* hrb_api(int edi, int esi, int ebp, int esp, int ebx, int edx, int ecx, int eax) {
  (void)edi; (void)esi; (void)ebp; (void)esp; (void)ebx; (void)edx; (void)ecx; (void)eax;
  TASK* task = task_now();
  const int ds_base = task->ds_base;  // Get data segment address.
  CONSOLE* cons = task->cons;
  volatile int* reg = &eax + 1;  // Need `volatile` modifier for gcc.
  // reg[0] = edi, reg[1] = esi, reg[2] = ebp, reg[3] = esp
  // reg[4] = ebx, reg[5] = edx, reg[6] = ecx, reg[7] = eax

  switch (edx) {
  case API_PUTCHAR:  cons_putchar(cons, eax & 0xff, TRUE); break;
  case API_PUTSTR0:  cons_putstr0(cons, (char*)ebx + ds_base); break;
  case API_PUTSTR1:  cons_putstr1(cons, (char*)ebx + ds_base, ecx); break;
  case API_END: return &task->tss.esp0;
  case API_OPENWIN:
    {
      unsigned char* buf = (unsigned char*)ebx + ds_base;
      int xsize = esi, ysize = edi, col_inv = eax;
      const char* title = (const char*)ecx + ds_base;
      SHTCTL* shtctl = (SHTCTL*)*((int*)0x0fe4);
      SHEET* sht = sheet_alloc(shtctl);
      sht->task = task;
      sht->flags |= 0x10;
      reg[7] = (int)sht;  // Set return value: SHEET* sheet == int win;
      sheet_setbuf(sht, buf, xsize, ysize, col_inv);
      make_window8(buf, xsize, ysize, title, FALSE);
      sheet_slide(shtctl, sht, (shtctl->xsize - xsize) / 2, (shtctl->ysize - ysize) / 2);
      sheet_updown(shtctl, sht, shtctl->top);
    }break;
  case API_PUTSTRWIN:
    {
      SHEET* sht = (SHEET*)(ebx & -2);  // SHEET* sheet == int win;
      char refresh = (ebx & 1) == 0;
      int x = esi, y = edi, col = eax, len = ecx;
      const char* str = (const char*)ebp + ds_base;
      putfonts8_asc(sht->buf, sht->bxsize, x, y, col, str);
      if (refresh) {
        SHTCTL* shtctl = (SHTCTL*)*((int*)0x0fe4);
        sheet_refresh(shtctl, sht, x, y, x + len * 8, y + 16);
      }
    }break;
  case API_BOXFILWIN:
    {
      SHEET* sht = (SHEET*)(ebx & -2);  // SHEET* sheet == int win;
      char refresh = (ebx & 1) == 0;
      int x0 = eax, y0 = ecx, x1 = esi, y1 = edi, col = ebp;
      boxfill8(sht->buf, sht->bxsize, col, x0, y0, x1, y1);
      if (refresh) {
        SHTCTL* shtctl = (SHTCTL*)*((int*)0x0fe4);
        sheet_refresh(shtctl, sht, x0, y0, x1, y1);
      }
    }break;
  case API_INITMALLOC:
    {
      MEMMAN* memman = (MEMMAN*)(ebx + ds_base);
      void* addr = (void*)eax;
      int size = ecx & -16;
      memman_init(memman);
      memman_free(memman, addr, size);
    }break;
  case API_MALLOC:
    {
      MEMMAN* memman = (MEMMAN*)(ebx + ds_base);
      int size = (ecx + 0x0f) & -16;  // Align with 16 bytes.
      reg[7] = (int)memman_alloc(memman, size);
    }break;
  case API_FREE:
    {
      MEMMAN* memman = (MEMMAN*)(ebx + ds_base);
      void* addr = (void*)eax;
      int size = (ecx + 0x0f) & -16;  // Align with 16 bytes.
      memman_free(memman, addr, size);
    }break;
  case API_POINT:
    {
      SHEET* sht = (SHEET*)(ebx & -2);  // SHEET* sheet == int win;
      char refresh = (ebx & 1) == 0;
      int x = esi, y = edi, col = eax;
      sht->buf[sht->bxsize * y + x] = col;
      if (refresh) {
        SHTCTL* shtctl = (SHTCTL*)*((int*)0x0fe4);
        sheet_refresh(shtctl, sht, x, y, x + 1, y + 1);
      }
    }break;
  case API_REFRESHWIN:
    {
      SHEET* sht = (SHEET*)ebx;  // SHEET* sheet == int win;
      int x0 = eax, y0 = ecx, x1 = esi, y1 = edi;
      SHTCTL* shtctl = (SHTCTL*)*((int*)0x0fe4);
      sheet_refresh(shtctl, sht, x0, y0, x1, y1);
    }break;
  case API_LINEWIN:
    {
      SHEET* sht = (SHEET*)(ebx & -2);  // SHEET* sheet == int win;
      char refresh = ebx & 1;
      int x0 = eax, y0 = ecx, x1 = esi, y1 = edi, col = ebp;
      line8(sht->buf, sht->bxsize, x0, y0, x1, y1, col);
      if (refresh) {
#define SWAP(type, a, b)  do { type tmp = a; a = b; b = tmp; } while (0)
        if (x0 > x1)
          SWAP(int, x0, x1);
        if (y0 > y1)
          SWAP(int, y0, y1);
        SHTCTL* shtctl = (SHTCTL*)*((int*)0x0fe4);
        sheet_refresh(shtctl, sht, x0, y0, x1, y1);
      }
    }break;
  case API_CLOSEWIN:
    {
      SHEET* sht = (SHEET*)ebx;  // SHEET* sheet == int win;
      SHTCTL* shtctl = (SHTCTL*)*((int*)0x0fe4);
      sheet_free(shtctl, sht);
    }break;
  case API_GETKEY:
    {
      int sleep = eax;
      for (;;) {
        io_cli();
        if (fifo_empty(&task->fifo)) {
          if (sleep) {
            task_sleep(task);
          } else {
            io_sti();
            reg[7] = -1;
            return NULL;
          }
        }
        int i = fifo_get(&task->fifo);
        io_sti();
        switch (i) {
        case 0: case 1:  // Cursor
          timer_init(cons->timer, &task->fifo, 1);  // Next disp.
          timer_settime(cons->timer, 50);
          break;
        case 2:  // Cursor on
          cons->cur_c = COL8_WHITE;
          break;
        case 3:  // Cursor off
          cons->cur_c = -1;
          break;
        case 4:  // Close console only.
          {
            SHTCTL* shtctl = (SHTCTL*)*((int*)0x0fe4);
            FIFO* sys_fifo = (FIFO*)*((int*)0x0fec);
            timer_cancel(cons->timer);
            io_cli();
            fifo_put(sys_fifo, cons->sheet - shtctl->sheets0 + 2024);
            cons->sheet = NULL;
            io_sti();
          }
          break;
        default:
          if (i >= 256) {  // Keyboard, etc.
            reg[7] = i - 256;
            return NULL;
          }
        }
      }
    }break;
  case API_ALLOCTIMER:
    {
      TIMER* timer = timer_alloc();
      timer->flags2 = 1;  // Enable auto cancel.
      reg[7] = (int)timer;
    }
    break;
  case API_INITTIMER:
    {
      TIMER* timer = (TIMER*)ebx;
      int data = eax;
      timer_init(timer, &task->fifo, data + 256);
    }
    break;
  case API_SETTIMER:
    {
      TIMER* timer = (TIMER*)ebx;
      int time = eax;
      timer_settime(timer, time);
    }
    break;
  case API_FREETIMER:
    {
      TIMER* timer = (TIMER*)ebx;
      timer_free(timer);
    }
    break;
  case API_BEEP:
    if (eax == 0) {
      int i = io_in8(0x61);
      io_out8(0x61, i & 0x0d);
    } else {
      int i = 1193180000 / eax;
      io_out8(0x43, 0xb6);
      io_out8(0x42, i & 0xff);
      io_out8(0x42, i >> 8);
      i = io_in8(0x61);
      io_out8(0x61, (i | 0x03) & 0x0f);
    }
    break;
  case API_FOPEN:
    reg[7] = 0;
    for (int i = 0; i < 8; ++i) {
      if (task->fhandle[i].buf == NULL) {
        const char* filename = (char*)ebx + ds_base;
        FILEHANDLE* fh = &task->fhandle[i];
        FILEINFO* finfo = file_search(filename, (FILEINFO*)(ADR_DISKIMG + 0x002600), 224);
        if (finfo != NULL) {
          MEMMAN* memman = (MEMMAN*)MEMMAN_ADDR;
          reg[7] = (int)fh;
          fh->buf = memman_alloc_4k(memman, finfo->size);
          fh->size = finfo->size;
          fh->pos = 0;
          file_loadfile(finfo->clustno, finfo->size, fh->buf, task->fat, (char*)(ADR_DISKIMG + 0x003e00));
        }
        break;
      }
    }
    break;
  case API_FCLOSE:
    {
      FILEHANDLE* fh = (FILEHANDLE*)eax;
      MEMMAN* memman = (MEMMAN*)MEMMAN_ADDR;
      memman_free_4k(memman, fh->buf, fh->size);
      fh->buf = NULL;
    }
    break;
  case API_FSEEK:
    {
      FILEHANDLE* fh = (FILEHANDLE*)eax;
      int origin = ecx;
      int offset = ebx;
      switch (origin) {
      case 0:  fh->pos = offset; break;
      case 1:  fh->pos += offset; break;
      case 2:  fh->pos = fh->size + offset; break;
      }
      if (fh->pos < 0)
        fh->pos = 0;
      else if (fh->pos > fh->size)
        fh->pos = fh->size;
    }
    break;
  case API_FSIZE:
    {
      FILEHANDLE* fh = (FILEHANDLE*)eax;
      int mode = ecx;
      switch (mode) {
      case 0:  reg[7] = fh->size; break;
      case 1:  reg[7] = fh->pos; break;
      case 2:  reg[7] = fh->pos - fh->size; break;
      }
    }
    break;
  case API_FREAD:
    {
      FILEHANDLE* fh = (FILEHANDLE*)eax;
      unsigned char* dst = (unsigned char*)ebx + ds_base;
      int size = ecx;
      unsigned char* src = &fh->buf[fh->pos];

      int readsize = (size > fh->size - fh->pos) ? fh->size - fh->pos : size;
      memcpy(dst, src, readsize);
      fh->pos += readsize;
      reg[7] = readsize;
    }
    break;
  case API_CMDLINE:
    {
      char* buf = (char*)ebx + ds_base;
      int maxsize = ecx;
      char* src = task->cmdline;
      int i;
      for (i = 0; i < maxsize && (*buf++ = *src++) != '\0'; ++i)
        ;
      reg[7] = i;
    }
    break;
  }
  return NULL;
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
  file_loadfile(finfo->clustno, finfo->size, p, fat, (char*)(ADR_DISKIMG + 0x003e00));
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
      if (task->fhandle[i].buf != NULL) {
        memman_free_4k(memman, task->fhandle[i].buf, task->fhandle[i].size);
        task->fhandle[i].buf = NULL;
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

void cons_runcmd(const char* cmdline, CONSOLE* cons, const short* fat, int memtotal) {
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

void console_task(SHTCTL* shtctl, SHEET* sheet, unsigned int memtotal) {
  TASK* task = task_now();

  MEMMAN *memman = (MEMMAN*) MEMMAN_ADDR;
  short* fat = (short*)memman_alloc_4k(memman, sizeof(short) * 2880);
  file_readfat(fat, (unsigned char*)(ADR_DISKIMG + 0x000200));

  FILEHANDLE fhandle[8];
  for (int i = 0; i < 8; ++i)
    fhandle[i].buf = NULL;  // Not used.
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
      switch (i) {
      case 10 + 256:  // Enter.
        // Erase cursor and newline.
        cons_putchar(&cons, ' ', FALSE);
        cmdline[cons.cur_x / 8 - 2] = '\0';
        cons_newline(&cons);
        cons_runcmd(cmdline, &cons, fat, memtotal);
        if (cons.sheet == NULL)
          cmd_exit(&cons, fat);
        cons_putchar(&cons, '>', TRUE);
        break;
      case 8 + 256:  // Back space.
        if (cons.cur_x > 16) {
          cons_putchar(&cons, ' ', FALSE);
          cons.cur_x -= 8;
        }
        break;
      default:  // Normal character.
        if (cons.cur_x < 240) {
          cmdline[cons.cur_x / 8 - 2] = i - 256;
          cons_putchar(&cons, i - 256, TRUE);
        }
        break;
      }
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
