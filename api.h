#ifndef __API_H__
#define __API_H__

void api_putchar(int c);
void api_putstr0(const char* s);
void api_end(void);
int api_openwin(unsigned char* buf, int xsiz, int ysiz, int col_inv, const char* title);
void api_putstrwin(int win, int x, int y, int col, int len, const char* str);
void api_boxfilwin(int win, int x0, int y0, int x1, int y1, int col);
void api_point(int win, int x, int y, int col);
void api_initmalloc(void);
void* api_malloc(int size);
void api_free(void* addr, int size);


void api_dumphex(int val);

#define RAND_MAX  (0x7fff)
int rand(void);

#endif
