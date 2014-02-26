#ifndef __STDIO_H__
#define __STDIO_H__

#include "stddef.h"

#ifdef __cplusplus
extern "C" {
#endif

int putchar(int c);
int puts(const char* str);
int printf(const char* format, ...);
int sprintf(char *str, const char *fmt, ...);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif
