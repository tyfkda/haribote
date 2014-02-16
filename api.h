#ifndef __API_H__
#define __API_H__

#define COL8_BLACK  (0)
#define COL8_RED  (1)
#define COL8_GREEN  (2)
#define COL8_YELLOW  (3)
#define COL8_BLUE  (4)
#define COL8_PURPLE  (5)
#define COL8_CYAN  (6)
#define COL8_WHITE  (7)
#define COL8_GRAY  (8)
#define COL8_DARK_RED  (9)
#define COL8_DARK_GREEN  (10)
#define COL8_DARK_YELLOW  (11)
#define COL8_DARK_BLUE  (12)
#define COL8_DARK_PURPLE  (13)
#define COL8_DARK_CYAN  (14)
#define COL8_DARK_GRAY  (15)

void api_putchar(int c);
void api_putstr0(const char* s);
void api_end(void);
int api_openwin(unsigned char* buf, int xsiz, int ysiz, int col_inv, const char* title);
void api_closewin(int win);
void api_putstrwin(int win, int x, int y, int col, int len, const char* str);
void api_boxfilwin(int win, int x0, int y0, int x1, int y1, int col);
void api_point(int win, int x, int y, int col);
void api_refresh(int win, int x0, int y0, int x1, int y1);
void api_linewin(int win, int x0, int y0, int x1, int y1, int col);
int api_getkey(int mode);
void api_initmalloc(void);
void* api_malloc(int size);
void api_free(void* addr, int size);

typedef struct TIMER TIMER;
TIMER* api_alloctimer(void);
void api_inittimer(TIMER* timer, int data);
void api_settimer(TIMER* timer, int time);
void api_freetimer(TIMER* timer);

void api_beep(int tone);


void api_dumphex(int val);

#define RAND_MAX  (0x7fff)
int rand(void);

int sprintf(char *str, const char *fmt, ...);

#endif
