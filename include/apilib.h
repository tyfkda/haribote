#ifndef __API_H__
#define __API_H__

#ifdef __cplusplus
extern "C" {
#endif

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

#define KEY_UP       (0x80)
#define KEY_LEFT     (0x81)
#define KEY_RIGHT    (0x82)
#define KEY_DOWN     (0x83)
#define KEY_F1       (0x91)
#define KEY_F2       (0x92)
#define KEY_F3       (0x93)
#define KEY_F4       (0x94)
#define KEY_F5       (0x95)
#define KEY_F6       (0x96)
#define KEY_F7       (0x97)
#define KEY_F8       (0x98)
#define KEY_F9       (0x99)
#define KEY_F10      (0x9a)
#define KEY_F11      (0x9b)
#define KEY_F12      (0x9c)

void api_putstr0(const char* s);
void api_end(void) __attribute__((__noreturn__));
int api_openwin(unsigned char* buf, int xsiz, int ysiz, int col_inv, const char* title);
void api_closewin(int win);
void api_putstrwin(int win, int x, int y, int col, int len, const char* str);
void api_boxfilwin(int win, int x0, int y0, int x1, int y1, int col);
void api_point(int win, int x, int y, int col);
void api_refreshwin(int win, int x0, int y0, int x1, int y1);
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

int api_fopen(const char* fname);
void api_fclose(int fhandle);
void api_fseek(int fhandle, int offset, int mode);
int api_fsize(int fhandle, int mode);
int api_fread(void* buf, int maxsize, int fhandle);
int api_delete(const char* fname);
int api_now(unsigned char* buf);

int api_cmdline(char* buf, int maxsize);


int sprintf(char *str, const char *fmt, ...);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif
