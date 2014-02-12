#ifndef __CONSOLE_H__
#define __CONSOLE_H__

struct SHTCTL;
struct SHEET;

typedef struct {
  struct SHTCTL* shtctl;
  struct SHEET* sheet;
  int cur_x, cur_y, cur_c;
} CONSOLE;

void console_task(struct SHTCTL* shtctl, struct SHEET* sheet, unsigned int memtotal);
void cons_putchar(CONSOLE* cons, int chr, char move);
void cons_newline(CONSOLE* cons);
void cons_runcmd(const char* cmdline, CONSOLE* cons, const short* fat, int memtotal);

#endif

