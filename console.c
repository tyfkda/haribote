#include "console.h"
#include "bootpack.h"
#include "fifo.h"
#include "file.h"
#include "graphics.h"
#include "mtask.h"
#include "sheet.h"
#include "stdio.h"
#include "window.h"

static int cons_newline(int cursor_y, SHTCTL* shtctl, SHEET* sheet) {
  if (cursor_y < 28 + 112) {
    cursor_y += 16;
  } else {
    unsigned char* buf = sheet->buf;
    int bxsize = sheet->bxsize;
    // Scroll.
    for (int y = 28; y < 28 + 112; ++y)
      for (int x = 8; x < 8 + 240; ++x)
        buf[x + y * bxsize] = buf[x + (y + 16) * bxsize];
    // Erase last line.
    boxfill8(buf, bxsize, COL8_BLACK, 8, 28 + 112, 8 + 240, 28 + 112 + 16);
    sheet_refresh(shtctl, sheet, 8, 28, 8 + 240, 28 + 128);
  }
  return cursor_y;
}

void console_task(SHTCTL* shtctl, SHEET* sheet, unsigned int memtotal) {
  TASK* task = task_now();
  int fifobuf[128];
  fifo_init(&task->fifo, 128, fifobuf, task);

  TIMER* timer = timer_alloc();
  timer_init(timer, &task->fifo, 1);
  timer_settime(timer, 50);

  int cursor_x = 16, cursor_y = 28, cursor_c = -1;
  // Show prompt.
  putfonts8_asc_sht(shtctl, sheet, cursor_x - 8, cursor_y, COL8_WHITE, COL8_BLACK, ">", 1);

  MEMMAN *memman = (MEMMAN*) MEMMAN_ADDR;
  FILEINFO *finfo = (FILEINFO*)(ADR_DISKIMG + 0x002600);
  short* fat = (short*)memman_alloc_4k(memman, sizeof(short) * 2880);
  file_readfat(fat, (unsigned char*)(ADR_DISKIMG + 0x000200));

  char cmdline[30];

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
        putfonts8_asc_sht(shtctl, sheet, cursor_x, cursor_y, COL8_WHITE, COL8_BLACK, " ", 1);
        cmdline[cursor_x / 8 - 2] = '\0';
        cursor_y = cons_newline(cursor_y, shtctl, sheet);
        // Run command.
        if (strcmp(cmdline, "mem") == 0) {
          MEMMAN* memman = (MEMMAN*)MEMMAN_ADDR;
          char s[30];
          sprintf(s, "total %4dMB", memtotal / (1024 * 1024));
          putfonts8_asc_sht(shtctl, sheet, 8, cursor_y, COL8_WHITE, COL8_BLACK, s, strlen(s));
          cursor_y = cons_newline(cursor_y, shtctl, sheet);
          sprintf(s, "free %5dKB", memman_total(memman) / 1024);
          putfonts8_asc_sht(shtctl, sheet, 8, cursor_y, COL8_WHITE, COL8_BLACK, s, strlen(s));
          cursor_y = cons_newline(cursor_y, shtctl, sheet);
        } else if (strcmp(cmdline, "cls") == 0) {
          boxfill8(sheet->buf, sheet->bxsize, COL8_BLACK, 8, 28, 8 + 240, 28 + 128);
          sheet_refresh(shtctl, sheet, 8, 28, 8 + 240, 28 + 128);
          cursor_y = 28;
        } else if (strcmp(cmdline, "dir") == 0) {
          for (int x = 0; x < 224; ++x) {
            FILEINFO* p = &finfo[x];
            if (p->name[0] == 0x00)  // End of table.
              break;
            if (p->name[0] == 0xe5)  // Deleted file.
              continue;
            if ((p->type & 0x18) == 0) {
              char s[30];
              sprintf(s, "        .      %7d", p->size);
              strncpy(&s[0], (const char*)p->name, 8);
              strncpy(&s[9], (const char*)p->ext, 3);
              if (p->ext[0] == ' ')  // No file extension: remove dot.
                s[8] = ' ';
              putfonts8_asc_sht(shtctl, sheet, 8, cursor_y, COL8_WHITE, COL8_BLACK, s, 30);
              cursor_y = cons_newline(cursor_y, shtctl, sheet);
            }
          }
        } else if (strncmp(cmdline, "type ", 5) == 0) {
          char s[12];
          for (int y = 0; y < 11; ++y)
            s[y] = ' ';
          int y = 0;
          for (int x = 5; y < 11 && cmdline[x] != '\0'; ++x) {
            if (cmdline[x] == '.') {
              y = 8;
            } else {
              unsigned char c = cmdline[x];
              if ('a' <= c && c <= 'z')
                c -= 'a' - 'A';
              s[y++] = c;
            }
          }
          int x;
          for (x = 0; x < 224; ++x) {
            FILEINFO* p = &finfo[x];
            if (p->name[0] == 0x00)  // End of table.
              break;
            if ((p->type & 0x18) == 0) {
              if (strncmp((char*)p->name, s, 11) == 0)
                break;
            }
          }
          if (x < 224 && finfo[x].name[0] != 0x00) {  // File found.
            int size = finfo[x].size;
            char* p = (char*)memman_alloc_4k(memman, size);
            file_loadfile(finfo[x].clustno, size, p, fat, (char*)(ADR_DISKIMG + 0x003e00));
            cursor_x = 8;
            for (int i = 0; i < size; ++i) {
              switch (p[i]) {
              case 0x09:  // Tab.
                for (;;) {
                  putfonts8_asc_sht(shtctl, sheet, cursor_x, cursor_y, COL8_WHITE, COL8_BLACK, " ", 1);
                  cursor_x += 8;
                  if (cursor_x >= 8 + 240) {
                    cursor_x = 8;
                    cursor_y = cons_newline(cursor_y, shtctl, sheet);
                  }
                  if (((cursor_x - 8) & 0x1f) == 0)
                    break;
                }
                break;
              case 0x0a:  // Line feed.
                cursor_x = 8;
                cursor_y = cons_newline(cursor_y, shtctl, sheet);
                break;
              case 0x0d:  // Carrige return.
                break;
              default:
                s[0] = p[i];
                s[1] = '\0';
                putfonts8_asc_sht(shtctl, sheet, cursor_x, cursor_y, COL8_WHITE, COL8_BLACK, s, 1);
                cursor_x += 8;
                if (cursor_x == 8 + 240) {
                  cursor_x = 8;
                  cursor_y = cons_newline(cursor_y, shtctl, sheet);
                }
                break;
              }
            }
            if (cursor_x != 8)
              cursor_y = cons_newline(cursor_y, shtctl, sheet);
            memman_free_4k(memman, (int)p, size);
          } else {
            putfonts8_asc_sht(shtctl, sheet, 8, cursor_y, COL8_WHITE, COL8_BLACK, "File not found", 14);
            cursor_y = cons_newline(cursor_y, shtctl, sheet);
          }
        } else if (cmdline[0] != '\0') {
          putfonts8_asc_sht(shtctl, sheet, 8, cursor_y, COL8_WHITE, COL8_BLACK, "Bad command.", 12);
          cursor_y = cons_newline(cursor_y, shtctl, sheet);
          cursor_y = cons_newline(cursor_y, shtctl, sheet);
        }
        // Show prompt.
        cursor_x = 16;
        putfonts8_asc_sht(shtctl, sheet, cursor_x - 8, cursor_y, COL8_WHITE, COL8_BLACK, ">", 1);
        break;
      case 8 + 256:  // Back space.
        if (cursor_x > 16) {
          putfonts8_asc_sht(shtctl, sheet, cursor_x, cursor_y, COL8_WHITE, COL8_BLACK, " ", 1);
          cursor_x -= 8;
        }
        break;
      default:  // Normal character.
        if (cursor_x < 240) {
          char s[] = { i - 256, '\0' };
          cmdline[cursor_x / 8 - 2] = s[0];
          putfonts8_asc_sht(shtctl, sheet, cursor_x, cursor_y, COL8_WHITE, COL8_BLACK, s, 1);
          cursor_x += 8;
        }
      }
    } else {
      switch (i) {
      case 0:
      case 1:
        if (cursor_c >= 0)
          cursor_c = i == 0 ? COL8_WHITE : COL8_BLACK;
        timer_init(timer, &task->fifo, 1 - i);
        timer_settime(timer, 50);
        break;
      case 2:  cursor_c = COL8_WHITE; break;
      case 3:
        boxfill8(sheet->buf, sheet->bxsize, COL8_BLACK, cursor_x, cursor_y, cursor_x + 8, cursor_y + 16);
        cursor_c = -1;
        break;
      }
    }
    // Redraw cursor.
    if (cursor_c >= 0) {
      boxfill8(sheet->buf, sheet->bxsize, cursor_c, cursor_x, cursor_y, cursor_x + 8, cursor_y + 16);
      sheet_refresh(shtctl, sheet, cursor_x, cursor_y, cursor_x + 8, cursor_y + 16);
    }
  }
}
