#ifndef __STRING_H__
#define __STRING_H__

int toupper(int c);
void* memset(void* buf, int ch, int n);
void* memcpy(void* dst, const void* src, int size);
int strlen(const char* str);
char* strcpy(char* dst, const char* src);
char* strncpy(char* dst, const char* src, int n);
int strcmp(const char* s1, const char* s2);
int strncmp(const char* s1, const char* s2, int n);

#endif
