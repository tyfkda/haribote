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
    memcpy(&buf[y * bxsize], &buf[(y + 16) * bxsize], 240);
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

void hrb_api(int edi, int esi, int ebp, int esp, int ebx, int edx, int ecx, int eax) {
  (void)edi; (void)esi; (void)ebp; (void)esp; (void)ebx; (void)edx; (void)ecx; (void)eax;
  const int cs_base = *((int*)0x0fe8);  // Get code segment address.
  CONSOLE* cons = (CONSOLE*)*((int*)0x0fec);
  switch (edx) {
  case 1:  cons_putchar(cons, eax & 0xff, TRUE); break;
  case 2:  cons_putstr0(cons, (char*)ebx + cs_base); break;
  case 3:  cons_putstr1(cons, (char*)ebx + cs_base, ecx); break;
  }
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
  memman_free_4k(memman, (int)p, size);
}

static char cmd_app(const short* fat, const char* cmdline) {
  char name[13];
  strncpy(name, cmdline, 8);
  name[8] = '\0';

  FILEINFO *finfo = file_search(name, (FILEINFO*)(ADR_DISKIMG + 0x002600), 224);
  if (finfo == NULL) {
    // Try executable extension.
    strcpy(name + strlen(name), ".HRB");
    finfo = file_search(name, (FILEINFO*)(ADR_DISKIMG + 0x002600), 224);
  }
  if (finfo == NULL)
    return FALSE;

  MEMMAN *memman = (MEMMAN*) MEMMAN_ADDR;
  char* p = (char*)memman_alloc_4k(memman, finfo->size);
  file_loadfile(finfo->clustno, finfo->size, p, fat, (char*)(ADR_DISKIMG + 0x003e00));

  SEGMENT_DESCRIPTOR* gdt = (SEGMENT_DESCRIPTOR*)ADR_GDT;
  set_segmdesc(gdt + 1003, finfo->size - 1, (int)p, AR_CODE32_ER);
  *((int*)0x0fe8) = (int)p;  // Store code segment address.
  farcall(0, 1003 * 8);
  memman_free_4k(memman, (int)p, finfo->size);
  return TRUE;
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
    if (!cmd_app(fat, cmdline))
      cons_putstr0(cons, "Bad command.\n");
  }
}

void console_task(SHTCTL* shtctl, SHEET* sheet, unsigned int memtotal) {
  TASK* task = task_now();
  int fifobuf[128];
  fifo_init(&task->fifo, 128, fifobuf, task);

  TIMER* timer = timer_alloc();
  timer_init(timer, &task->fifo, 1);
  timer_settime(timer, 50);

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
        timer_init(timer, &task->fifo, 1 - i);
        timer_settime(timer, 50);
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
