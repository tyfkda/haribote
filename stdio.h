#ifndef __STDIO_H__
#define __STDIO_H__

#define FALSE  (0)
#define TRUE   (1)
#define NULL   ((void*)0)

int strlen(const char* str);
int strcmp(const char* s1, const char* s2);
char* strcpy(char* dst, char* src);
int sprintf(char *str, const char *fmt, ...);

#endif