#ifndef __STDIO_H__
#define __STDIO_H__

#include "stddef.h"
#include "stdint.h"

#ifdef __cplusplus
extern "C" {
#endif

#define EOF  (-1)

typedef struct FILE FILE;

extern FILE *stdin, *stdout, *stderr;

int putchar(int c);
int puts(const char* str);
int printf(const char* format, ...);
int sprintf(char *str, const char *fmt, ...);

FILE* fopen(const char* filename, const char* mode);
void fclose(FILE* fp);
int fprintf(FILE* stream, const char* format, ...);
size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream);
size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream);
int fgetc(FILE *stream);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif
