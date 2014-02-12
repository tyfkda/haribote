#ifndef __STDIO_H__
#define __STDIO_H__

#define FALSE  (0)
#define TRUE   (1)
#define NULL   ((void*)0)

int toupper(int c);
void* memset(void* buf, int ch, int n);
void* memcpy(void* dst, const void* src, int size);
int strlen(const char* str);
char* strcpy(char* dst, const char* src);
char* strncpy(char* dst, const char* src, int n);
int strcmp(const char* s1, const char* s2);
int strncmp(const char* s1, const char* s2, int n);
int sprintf(char *str, const char *fmt, ...);

#endif
