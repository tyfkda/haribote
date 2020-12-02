#ifndef __STRING_H__
#define __STRING_H__

#include "stdint.h"

#ifdef __cplusplus
extern "C" {
#endif

void* memset(void* buf, int ch, size_t n);
void* memcpy(void* __restrict__ dst, const void* __restrict__ src, size_t size);
void* memmove(void* dst, const void* src, size_t size);
int memcmp(const void *s1, const void *s2, size_t n);
void *memchr(const void *s, int c, size_t n);

size_t strlen(const char* str);
char* strcpy(char* dst, const char* src);
char* strncpy(char* dst, const char* src, size_t n);
char *strcat(char *dest, const char *src);
char *strchr(const char *s, int c);
char *strstr(const char *haystack, const char *needle);
int strcmp(const char* s1, const char* s2);
int strncmp(const char* s1, const char* s2, size_t n);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif
