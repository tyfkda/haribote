#include "hrb_api.h"
#include "apilib.h"
#include "bootpack.h"
#include "console.h"
#include "fd.h"
#include "graphics.h"
#include "memory.h"
#include "mtask.h"
#include "sheet.h"
#include "timer.h"
#include "window.h"
#include "../apilib/apisrc/stdio_def.h"

#define SWAP(type, a, b)  do { type tmp = a; a = b; b = tmp; } while (0)

static int bcd2(unsigned char x) {
  return (x >> 4) * 10 + (x & 0x0f);
}

static FDHANDLE* _api_fopen(TASK* task, const char* filename, int flag) {
  FDHANDLE* fh = task_get_free_fhandle(task);
  if (fh == NULL)
    return NULL;

  if (flag & OPEN_WRITE) {
    if (!fd_writeopen(fh, filename))
      return NULL;
  } else {
    if (!fd_open(fh, filename))
      return NULL;
  }
  return fh;
}

// Reads real time clock, and returns the result into array t.
// 0 = sec, 1 = min, 2 = hour, 3 = day, 4 = month, 5..6 = year
// Each value is represented in BCD. e.g. 12 = 0x12
static void read_rtc(unsigned char t[7]) {
  static const unsigned char adr[7] = { 0x00, 0x02, 0x04, 0x07, 0x08, 0x09, 0x32 };
  static const unsigned char max[7] = { 0x60, 0x59, 0x23, 0x31, 0x12, 0x99, 0x99 };
  for (int i = 0; i < 7; i++) {
    io_out8(0x70, adr[i]);
    unsigned char v = io_in8(0x71);
    if (!((v & 0x0f) <= 9 && v <= max[i]))
      v = -1;
    t[i] = v;
  }
  char err = FALSE;
  do {
    for (int i = 0; i < 7; i++) {
      io_out8(0x70, adr[i]);
      unsigned char v = io_in8(0x71);
      if (t[i] != v) {
        if ((v & 0x0f) <= 9 && v <= max[i])
          t[i] = v;
        err = TRUE;
      }
    }
  } while (err);
}

static int waitKeyInput(TASK* task, int sleep) {
  CONSOLE* cons = task->cons;
  for (;;) {
    io_cli();
    if (fifo_empty(&task->fifo)) {
      if (sleep) {
        task_sleep(task);
      } else {
        io_sti();
        return -1;
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
        SHTCTL* shtctl = getOsInfo()->shtctl;
        FIFO* sys_fifo = getOsInfo()->fifo;
        timer_cancel(cons->timer);
        io_cli();
        fifo_put(sys_fifo, cons->sheet - shtctl->sheets0 + 2024);
        cons->sheet = NULL;
        io_sti();
      }
      break;
    default:
      if (i >= 256) {  // Keyboard, etc.
        return i - 256;
      }
    }
  }
}

// This is the system call for Haribote OS.
// Called from assembler.
int* hrb_api(int edi, int esi, int ebp, int esp, int ebx, int edx, int ecx, int eax) {
  (void)edi; (void)esi; (void)ebp; (void)esp; (void)ebx; (void)edx; (void)ecx; (void)eax;
  TASK* task = task_now();
  const int ds_base = task->ds_base;  // Get data segment address.
  CONSOLE* cons = task->cons;
  volatile int* reg = &eax + 1;  // Need `volatile` modifier for gcc.
  // reg[0] = edi, reg[1] = esi, reg[2] = ebp, reg[3] = esp
  // reg[4] = ebx, reg[5] = edx, reg[6] = ecx, reg[7] = eax

  switch (edx) {
  case API_PUTCHAR:  cons_putchar(cons, eax & 0xff, TRUE, FALSE); break;
  case API_PUTSTR0:  cons_putstr0(cons, (char*)eax + ds_base); break;
  case API_PUTSTR1:  cons_putstr1(cons, (char*)eax + ds_base, ecx); break;
  case API_END: return &task->tss.esp0;
  case API_OPENWIN:
    {
      unsigned char* buf = (unsigned char*)ebx + ds_base;
      int xsize = esi, ysize = edi, col_inv = eax;
      const char* title = (const char*)ecx + ds_base;
      SHTCTL* shtctl = getOsInfo()->shtctl;
      SHEET* sheet = sheet_alloc(shtctl);
      sheet->task = task;
      sheet->flags |= 0x10;
      reg[7] = (int)sheet;  // Set return value: SHEET* sheet == int win;
      sheet_setbuf(sheet, buf, xsize, ysize, col_inv);
      make_window8(sheet, title, FALSE);
      sheet_slide(shtctl, sheet, (shtctl->xsize - xsize) / 2, (shtctl->ysize - ysize) / 2);
      sheet_updown(shtctl, sheet, shtctl->top);
    }break;
  case API_PUTSTRWIN:
    {
      SHEET* sheet = (SHEET*)(ebx & ~1);  // SHEET* sheet == int win;
      char refresh = (ebx & 1) == 0;
      int x = esi, y = edi, col = eax, len = ecx;
      const char* str = (const char*)ebp + ds_base;
      putfonts8_asc(sheet->buf, sheet->bxsize, sheet->bysize, x, y, col, str);
      if (refresh) {
        SHTCTL* shtctl = getOsInfo()->shtctl;
        sheet_refresh(shtctl, sheet, x, y, x + len * 8, y + 16);
      }
    }break;
  case API_BOXFILWIN:
    {
      SHEET* sheet = (SHEET*)(ebx & ~1);  // SHEET* sheet == int win;
      char refresh = (ebx & 1) == 0;
      int x0 = eax, y0 = ecx, x1 = esi, y1 = edi, col = ebp;
      boxfill8(sheet->buf, sheet->bxsize, col, x0, y0, x1, y1);
      if (refresh) {
        SHTCTL* shtctl = getOsInfo()->shtctl;
        sheet_refresh(shtctl, sheet, x0, y0, x1, y1);
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
      MEMMAN* memman = (MEMMAN*)(ecx + ds_base);
      int size = (eax + 0x0f) & -16;  // Align with 16 bytes.
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
      SHEET* sheet = (SHEET*)(ecx & ~1);  // SHEET* sheet == int win;
      char refresh = (ecx & 1) == 0;
      int x = esi, y = edi, col = eax;
      sheet->buf[sheet->bxsize * y + x] = col;
      if (refresh) {
        SHTCTL* shtctl = getOsInfo()->shtctl;
        sheet_refresh(shtctl, sheet, x, y, x + 1, y + 1);
      }
    }break;
  case API_REFRESHWIN:
    {
      SHEET* sheet = (SHEET*)ebx;  // SHEET* sheet == int win;
      int x0 = eax, y0 = ecx, x1 = esi, y1 = edi;
      SHTCTL* shtctl = getOsInfo()->shtctl;
      sheet_refresh(shtctl, sheet, x0, y0, x1, y1);
    }break;
  case API_LINEWIN:
    {
      SHEET* sheet = (SHEET*)(ebx & ~1);  // SHEET* sheet == int win;
      char refresh = ebx & 1;
      int x0 = eax, y0 = ecx, x1 = esi, y1 = edi, col = ebp;
      line8(sheet->buf, sheet->bxsize, x0, y0, x1, y1, col);
      if (refresh) {
        if (x0 > x1)
          SWAP(int, x0, x1);
        if (y0 > y1)
          SWAP(int, y0, y1);
        SHTCTL* shtctl = getOsInfo()->shtctl;
        sheet_refresh(shtctl, sheet, x0, y0, x1, y1);
      }
    }break;
  case API_CLOSEWIN:
    {
      SHEET* sheet = (SHEET*)eax;  // SHEET* sheet == int win;
      SHTCTL* shtctl = getOsInfo()->shtctl;
      sheet_free(shtctl, sheet);
    }break;
  case API_GETKEY:
    {
      int sleep = eax;
      reg[7] = waitKeyInput(task, sleep);
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
      TIMER* timer = (TIMER*)ecx;
      int data = eax;
      timer_init(timer, &task->fifo, data + 256);
    }
    break;
  case API_SETTIMER:
    {
      TIMER* timer = (TIMER*)ecx;
      int time = eax;
      timer_settime(timer, time);
    }
    break;
  case API_FREETIMER:
    {
      TIMER* timer = (TIMER*)eax;
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
    {
      const char* filename = (char*)ecx + ds_base;
      int flag = eax;
      reg[7] = (int)_api_fopen(task, filename, flag);
    }
    break;
  case API_FCLOSE:
    {
      FDHANDLE* fh = (FDHANDLE*)eax;
      fd_close(fh);
    }
    break;
  case API_FSEEK:
    {
      FDHANDLE* fh = (FDHANDLE*)eax;
      int origin = ecx;
      int offset = ebx;
      fd_seek(fh, offset, origin);
    }
    break;
  case API_FSIZE:
    {
      FDHANDLE* fh = (FDHANDLE*)eax;
      int mode = ecx;
      switch (mode) {
      case 0:  reg[7] = fh->finfo->size; break;
      case 1:  reg[7] = fh->pos; break;
      case 2:  reg[7] = fh->pos - fh->finfo->size; break;
      }
    }
    break;
  case API_FREAD:
    {
      int handle = eax;
      unsigned char* dst = (unsigned char*)ebx + ds_base;
      int size = ecx;
      if (handle == kSTDIN) {
        int readSize = 0;
        for (; size > 0; --size, ++readSize) {
          int c = waitKeyInput(task, TRUE);
          if (c < 0)
            break;
          *dst++ = c;
        }
        reg[7] = readSize;
      } else {
        FDHANDLE* fh = (FDHANDLE*)handle;
        int readSize = fd_read(fh, dst, size);
        reg[7] = readSize;
      }
    }
    break;
  case API_FWRITE:
    {
      int handle = eax;
      const void* src = (unsigned char*)ebx + ds_base;
      int size = ecx;
      if (handle == kSTDOUT || handle == kSTDERR) {
        cons_putstr1(cons, src, size);
      } else {
        FDHANDLE* fh = (FDHANDLE*)handle;
        int writesize = fd_write(fh, src, size);
        reg[7] = writesize;
      }
    }
    break;
  case API_CMDLINE:
    {
      char* buf = (char*)ecx + ds_base;
      int maxsize = eax;
      char* src = task->cmdline;
      int i;
      for (i = 0; i < maxsize && (*buf++ = *src++) != '\0'; ++i)
        ;
      reg[7] = i;
    }
    break;
  case API_DELETE:
    {
      const char* filename = (char*)eax + ds_base;
      reg[7] = fd_delete(filename);
    }
    break;
  case API_NOW:
    {
      unsigned char* buf = (unsigned char*)eax + ds_base;
      unsigned char t[7];
      read_rtc(t);
      short year = bcd2(t[6]) * 100 + bcd2(t[5]);
      unsigned char month = bcd2(t[4]);
      unsigned char day = bcd2(t[3]);
      unsigned char hour = bcd2(t[2]);
      unsigned char minute = bcd2(t[1]);
      unsigned char second = bcd2(t[0]);
      buf[0] = year >> 8;
      buf[1] = year;
      buf[2] = month;
      buf[3] = day;
      buf[4] = hour;
      buf[5] = minute;
      buf[6] = second;
    }
    break;
  }
  return NULL;
}
