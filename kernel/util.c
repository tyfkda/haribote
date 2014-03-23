#include "util.h"
#include "bootpack.h"
#include "console.h"
#include "fd.h"
#include "mtask.h"
#include "sheet.h"
#include "stdio.h"
#include "string.h"
#include "timer.h"

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

static int bcd2(unsigned char x) {
  return (x >> 4) * 10 + (x & 0x0f);
}

int read_rtc(unsigned char tt[5]) {
  static const unsigned char adr[7] = { 0x00, 0x02, 0x04, 0x07, 0x08, 0x09, 0x32 };
  static const unsigned char max[7] = { 0x60, 0x59, 0x23, 0x31, 0x12, 0x99, 0x99 };
  unsigned char t[7];
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

  int year = bcd2(t[6]) * 100 + bcd2(t[5]);
  tt[0] = bcd2(t[4]);  // month.
  tt[1] = bcd2(t[3]);  // day.
  tt[2] = bcd2(t[2]);  // hour.
  tt[3] = bcd2(t[1]);  // minute.
  tt[4] = bcd2(t[0]);  // second.
  return year;
}

void cmd_mem(CONSOLE* cons) {
  unsigned int memtotal = getOsInfo()->memtotal;
  MEMMAN* memman = (MEMMAN*)MEMMAN_ADDR;
  cons_printf(cons, "total %4udMB\nfree %5dKB\n",
              memtotal / (1024 * 1024),
              memman_total(memman) / 1024);
}

void cmd_dir(CONSOLE* cons) {
  FDINFO *finfo = (FDINFO*)(ADR_DISKIMG + 0x002600);
  for (int i = 0; i < 224; ++i) {
    FDINFO* p = &finfo[i];
    if (p->name[0] == 0x00)  // End of table.
      break;
    if (p->name[0] == 0xe5)  // Deleted file.
      continue;
    if ((p->type & 0x18) == 0) {
      char name[8 + 1], ext[3 + 1];
      memcpy(name, p->name, 8); name[8] = '\0';
      memcpy(ext, p->ext, 3); ext[3] = '\0';
      int dot = (p->ext[0] != ' ') ? '.' : ' ';  // No file extension: remove dot.
      int year = ((p->date >> 9) & 0x7f) + 1980;
      int month = ((p->date >> 5) & 0x0f) + 1;
      int day = (p->date & 0x1f) + 1;
      int hour = (p->time >> 11) & 0x1f;
      int minute = (p->time >> 5) & 0x3f;
      cons_printf(cons, "%8s%c%3s   %7d '%02d/%02d/%02d %02d:%02d\n",
                  name, dot, ext, p->size, year % 100, month, day, hour, minute);
    }
  }
}

void cmd_exit(CONSOLE* cons) {
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

void cmd_start(const char* cmdline) {
  SHTCTL* shtctl = getOsInfo()->shtctl;
  SHEET* sheet = open_console(shtctl);
  sheet_slide(shtctl, sheet, 32, 4);
  sheet_updown(shtctl, sheet, shtctl->top);

  // Send key command.
  FIFO* fifo = &sheet->task->fifo;
  for (int i = 6; cmdline[i] != 0; ++i)
    fifo_put(fifo, cmdline[i] + 256);
  fifo_put(fifo, 10 + 256);  // Enter.
}

// No console start.
void cmd_ncst(const char* cmdline) {
  TASK* task = open_constask(NULL, NULL);

  // Send key command.
  FIFO* fifo = &task->fifo;
  for (int i = 5; cmdline[i] != 0; ++i)
    fifo_put(fifo, cmdline[i] + 256);
  fifo_put(fifo, 10 + 256);  // Enter.
}

void cmd_fat(struct CONSOLE* cons) {
  for (int j = 0; j < 16; ++j) {
    cons_printf(cons, "%04x:", j * 8);
    for (int i = 0; i < 8; ++i) {
      short c = get_next_cluster(j * 8 + i);
      cons_printf(cons, " %03x", c);
    }
    cons_putstr0(cons, "\n");
  }
}

void cmd_dir2(CONSOLE* cons) {
  FDINFO *finfo = (FDINFO*)(ADR_DISKIMG + 0x002600);
  for (int i = 0; i < 224; ++i) {
    FDINFO* p = &finfo[i];
    if (p->name[0] == 0x00)  // End of table.
      break;
    if (p->name[0] == 0xe5)  // Deleted file.
      continue;
    if ((p->type & 0x18) == 0) {
      char name[8 + 1], ext[3 + 1];
      memcpy(name, p->name, 8); name[8] = '\0';
      memcpy(ext, p->ext, 3); ext[3] = '\0';
      int dot = (p->ext[0] != ' ') ? '.' : ' ';  // No file extension: remove dot.
      cons_printf(cons, "%8s%c%3s   %3x\n", name, dot, ext, p->clustno);
    }
  }
}

char cmd_app(CONSOLE* cons, const char* cmdline) {
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
  size_t bssSize = (header.heapAdr - header.dataAdr) - header.dataSize;
  memset(data + header.esp + header.dataSize, 0x00, bssSize);

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
