#ifndef __STDIO_H__
#define __STDIO_H__

#define FALSE  (0)
#define TRUE   (1)
#define NULL   ((void*)0)

int strlen(const char* str);
char* strcpy(char* dst, const char* src);
char* strncpy(char* dst, const char* src, int n);
int strcmp(const char* s1, const char* s2);
int sprintf(char *str, const char *fmt, ...);

#endif
