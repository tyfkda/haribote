#include "console.h"
#include "bootpack.h"
#include "dsctbl.h"
#include "fifo.h"
#include "file.h"
#include "graphics.h"
#include "mtask.h"
#include "sheet.h"
#include "stdio.h"
#include "window.h"

void cons_newline(CONSOLE* cons) {
  cons->cur_x = 8;
  if (cons->cur_y < 28 + 112) {
    cons->cur_y += 16;
    return;
  }
  SHEET* sheet = cons->sheet;
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
  const int ds_base = *((int*)0x0fe8);  // Get data segment address.
  TASK* task = task_now();
  CONSOLE* cons = (CONSOLE*)*((int*)0x0fec);
  volatile int* reg = &eax + 1;  // Need `volatile` modifier for gcc.
  // reg[0] = edi, reg[1] = esi, reg[2] = ebp, reg[3] = esp
  // reg[4] = ebx, reg[5] = edx, reg[6] = ecx, reg[7] = eax

  switch (edx) {
  case 1:  cons_putchar(cons, eax & 0xff, TRUE); break;
  case 2:  cons_putstr0(cons, (char*)ebx + ds_base); break;
  case 3:  cons_putstr1(cons, (char*)ebx + ds_base, ecx); break;
  case 4: return &task->tss.esp0;
  case 5:
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
      sheet_slide(shtctl, sht, 100 + 200, 50);
      sheet_updown(shtctl, sht, 2);
    }break;
  case 6:
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
  case 7:
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
  case 8:
    {
      MEMMAN* memman = (MEMMAN*)(ebx + ds_base);
      void* addr = (void*)eax;
      int size = ecx & -16;
      memman_init(memman);
      memman_free(memman, addr, size);
    }break;
  case 9:
    {
      MEMMAN* memman = (MEMMAN*)(ebx + ds_base);
      int size = (ecx + 0x0f) & -16;  // Align with 16 bytes.
      reg[7] = (int)memman_alloc(memman, size);
    }break;
  case 10:
    {
      MEMMAN* memman = (MEMMAN*)(ebx + ds_base);
      void* addr = (void*)eax;
      int size = (ecx + 0x0f) & -16;  // Align with 16 bytes.
      memman_free(memman, addr, size);
    }break;
  case 11:
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
  case 12:
    {
      SHEET* sht = (SHEET*)ebx;  // SHEET* sheet == int win;
      int x0 = eax, y0 = ecx, x1 = esi, y1 = edi;
      SHTCTL* shtctl = (SHTCTL*)*((int*)0x0fe4);
      sheet_refresh(shtctl, sht, x0, y0, x1, y1);
    }break;
  case 13:
    {
      SHEET* sht = (SHEET*)(ebx & -2);  // SHEET* sheet == int win;
      char refresh = ebx & 1;
      int x0 = eax, y0 = ecx, x1 = esi, y1 = edi, col = ebp;
      line8(sht->buf, sht->bxsize, x0, y0, x1, y1, col);
      if (refresh) {
        SHTCTL* shtctl = (SHTCTL*)*((int*)0x0fe4);
        sheet_refresh(shtctl, sht, x0, y0, x1, y1);
      }
    }break;
  case 14:
    {
      SHEET* sht = (SHEET*)ebx;  // SHEET* sheet == int win;
      SHTCTL* shtctl = (SHTCTL*)*((int*)0x0fe4);
      sheet_free(shtctl, sht);
    }break;
  case 15:
    {
      int sleep = eax;
      for (;;) {
        io_cli();
        if (fifo_status(&task->fifo) == 0) {
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
        default:
          if (i >= 256) {  // Keyboard, etc.
            reg[7] = i - 256;
            return NULL;
          }
        }
      }
    }break;
  case 16:
    reg[7] = (int)timer_alloc();
    break;
  case 17:
    {
      TIMER* timer = (TIMER*)ebx;
      int data = eax;
      timer_init(timer, &task->fifo, data + 256);
    }
    break;
  case 18:
    {
      TIMER* timer = (TIMER*)ebx;
      int time = eax;
      timer_settime(timer, time);
    }
    break;
  case 19:
    {
      TIMER* timer = (TIMER*)ebx;
      timer_free(timer);
    }
    break;
  case 10000:  // dumphex
    {
      int val = eax;
      char s[30];
      sprintf(s, "Dump: %08x\n", val);
      cons_putstr0(cons, s);
    }break;
  case 10001:  // rand
    {
      static int rand_x;
      int a = 1103515245, b = 12345, c = 2147483647;
      rand_x = (a * rand_x + b) & c;
      reg[7] = (rand_x >> 16) & 0x7fff;  // RAND_MAX
    }break;
  case 10002:  // sprintf
    {
      char* buf = (char*)(ebx + ds_base);
      char* format = (char*)(ecx + ds_base);
      int* ap = (int*)(edi + ds_base);
      vsprintf(buf, format, ap);
    }break;
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

static void cmd_type(CONSOLE* cons, const short* fat, const char* cmdline) {
  MEMMAN *memman = (MEMMAN*) MEMMAN_ADDR;
  FILEINFO *finfo = file_search(cmdline + 5, (FILEINFO*)(ADR_DISKIMG + 0x002600), 224);
  if (finfo == NULL) {
    cons_putstr0(cons, "File not found.");
    return;
  }

  int size = finfo->size;
  char* p = (char*)memman_alloc_4k(memman, size);
  file_loadfile(finfo->clustno, size, p, fat, (char*)(ADR_DISKIMG + 0x003e00));
  cons_putstr1(cons, p, size);
  memman_free_4k(memman, p, size);
}

static char cmd_app(CONSOLE* cons, const short* fat, const char* cmdline) {
  char name[13];
  strncpy(name, cmdline, 8);
  name[8] = '\0';

  FILEINFO *finfo = file_search(name, (FILEINFO*)(ADR_DISKIMG + 0x002600), 224);
  if (finfo == NULL) {
    // Try executable extension.
    strcpy(name + strlen(name), ".HRB");
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

    SEGMENT_DESCRIPTOR* gdt = (SEGMENT_DESCRIPTOR*)ADR_GDT;
    set_segmdesc(gdt + 1003, finfo->size - 1, (int)p, AR_CODE32_ER + 0x60);
    set_segmdesc(gdt + 1004, 64 * 1024 - 1, (int)q, AR_DATA32_RW + 0x60);
    *((int*)0x0fe8) = (int)q;  // Store data segment address.
    memcpy(&q[esp], &p[dathrb], datsiz);

    TASK* task = task_now();
    start_app(0x1b, 1003 * 8, esp, 1004 * 8, &(task->tss.esp0));

    // End of application.
    // Free sheets which are opened by the task.
    SHTCTL* shtctl = (SHTCTL*)*((int*)0x0fe4);
    for (int i = 0; i < MAX_SHEETS; ++i) {
      SHEET* sht = &shtctl->sheets0[i];
      if ((sht->flags & 0x11) == 0x11 && sht->task == task)
        sheet_free(shtctl, sht);
    }
    memman_free_4k(memman, q, 64 * 1024);
  }
  memman_free_4k(memman, p, finfo->size);
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
  CONSOLE* cons = (CONSOLE*)*((int*)0x0fec);
  cons_putstr0(cons, "\nINT 0C :\n Stack Exception.\n");
  char s[30];
  sprintf(s, "EIP = %08x\n", esp[11]);
  cons_putstr0(cons, s);
  TASK* task = task_now();
  return &task->tss.esp0;  // Abort
}

int* inthandler0d(void) {
  CONSOLE* cons = (CONSOLE*)*((int*)0x0fec);
  cons_putstr0(cons, "\nINT 0D :\n General Protected Exception.\n");
  TASK* task = task_now();
  return &task->tss.esp0;  // Abort
}

void cons_runcmd(const char* cmdline, CONSOLE* cons, const short* fat, int memtotal) {
  if (strcmp(cmdline, "mem") == 0) {
    cmd_mem(cons, memtotal);
  } else if (strcmp(cmdline, "cls") == 0) {
    cmd_cls(cons);
  } else if (strcmp(cmdline, "dir") == 0) {
    cmd_dir(cons);
  } else if (strncmp(cmdline, "type ", 5) == 0) {
    cmd_type(cons, fat, cmdline);
  } else if (cmdline[0] != '\0') {
    if (!cmd_app(cons, fat, cmdline))
      cons_putstr0(cons, "Bad command.\n");
  }
}

void console_task(SHTCTL* shtctl, SHEET* sheet, unsigned int memtotal) {
  TASK* task = task_now();
  int fifobuf[128];
  fifo_init(&task->fifo, 128, fifobuf, task);

  MEMMAN *memman = (MEMMAN*) MEMMAN_ADDR;
  short* fat = (short*)memman_alloc_4k(memman, sizeof(short) * 2880);
  file_readfat(fat, (unsigned char*)(ADR_DISKIMG + 0x000200));

  char cmdline[30];

  // Show prompt.
  CONSOLE cons;
  cons.shtctl = shtctl;
  cons.sheet = sheet;
  cons.cur_x = 8;
  cons.cur_y = 28;
  cons.cur_c = -1;
  *((int*)0x0fec) = (int)&cons;
  cons_putchar(&cons, '>', TRUE);

  cons.timer = timer_alloc();
  timer_init(cons.timer, &task->fifo, 1);
  timer_settime(cons.timer, 50);

  for (;;) {
    io_cli();
    if (fifo_status(&task->fifo) == 0) {
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
        if (cons.cur_c >= 0)
          cons.cur_c = i == 0 ? COL8_WHITE : COL8_BLACK;
        timer_init(cons.timer, &task->fifo, 1 - i);
        timer_settime(cons.timer, 50);
        break;
      case 2:  cons.cur_c = COL8_WHITE; break;
      case 3:
        boxfill8(sheet->buf, sheet->bxsize, COL8_BLACK, cons.cur_x, cons.cur_y, cons.cur_x + 8, cons.cur_y + 16);
        cons.cur_c = -1;
        break;
      }
    }
    // Redraw cursor.
    if (cons.cur_c >= 0)
      boxfill8(sheet->buf, sheet->bxsize, cons.cur_c, cons.cur_x, cons.cur_y, cons.cur_x + 8, cons.cur_y + 16);
    sheet_refresh(shtctl, sheet, cons.cur_x, cons.cur_y, cons.cur_x + 8, cons.cur_y + 16);
  }
}
