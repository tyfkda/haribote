#ifndef __STRING_H__
#define __STRING_H__

#include "stdint.h"

#ifdef __cplusplus
extern "C" {
#endif

void* memset(void* buf, int ch, size_t n);
void* memcpy(void* restrict dst, const void* restrict src, size_t size);
void* memmove(void* dst, const void* src, size_t size);
int strlen(const char* str);
char* strcpy(char* dst, const char* src);
char* strncpy(char* dst, const char* src, size_t n);
int strcmp(const char* s1, const char* s2);
int strncmp(const char* s1, const char* s2, size_t n);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif
