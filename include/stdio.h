#ifndef __STDIO_H__
#define __STDIO_H__

#define FALSE  (0)
#define TRUE   (1)
#define NULL   ((void*)0)

int putchar(int c);
int puts(const char* str);
int printf(const char* format, ...);
int sprintf(char *str, const char *fmt, ...);

void exit(int status) __attribute__((__noreturn__));

void* malloc(int size);
void free(void* p);

// stdlib.h
long strtol(char *s, char **endp, int base);

#endif
