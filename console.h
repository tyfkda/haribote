#ifndef __CONSOLE_H__
#define __CONSOLE_H__

struct SHTCTL;
struct SHEET;

void console_task(struct SHTCTL* shtctl, struct SHEET* sheet, unsigned int memtotal);

#endif

