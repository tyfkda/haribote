#ifndef __UTIL_H__
#define __UTIL_H__

struct CONSOLE;

// Reads real time clock, and returns the result into array t.
// return value = year, 0=month, 1=day, 2=hour, 3=minute, 4=second
int read_rtc(unsigned char t[5]);

void cmd_mem(struct CONSOLE* cons);
void cmd_cls(struct CONSOLE* cons);
void cmd_dir(struct CONSOLE* cons);
void cmd_exit(struct CONSOLE* cons);
void cmd_start(char* argv[]);
void cmd_ncst(char* argv[]);
char cmd_app(struct CONSOLE* cons, char* argv[]);

void cmd_fat(struct CONSOLE* cons);
void cmd_dir2(struct CONSOLE* cons);

#endif
