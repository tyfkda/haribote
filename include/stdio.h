#ifndef __STDIO_H__
#define __STDIO_H__

#include "stddef.h"
#include "stdint.h"

#ifdef __cplusplus
extern "C" {
#endif

#define EOF  (-1)

// mode for fseek
#define SEEK_TOP  (0)
#define SEEK_CUR  (1)
#define SEEK_END  (2)

typedef struct FILE FILE;

extern FILE *stdin, *stdout, *stderr;

int putchar(int c);
int puts(const char* str);
int printf(const char* format, ...);
int sprintf(char *str, const char *format, ...);
int snprintf(char *str, size_t size, const char *format, ...);

FILE* fopen(const char* filename, const char* mode);
void fclose(FILE* fp);
int fseek(FILE *stream, long offset, int whence);
long ftell(FILE *stream);
int fputc(int c, FILE *stream);
int fputs(const char *s, FILE *stream);
int fprintf(FILE* stream, const char* format, ...);
size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream);
size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream);
int fgetc(FILE *stream);
char *fgets(char *s, int size, FILE *stream);

//int putc(int c, FILE *stream);
#define putc(c, stream)  fputc(c, stream)
#define getc(stream)  fgetc(stream)
#define getchar()  fgetc(stdin)

#ifdef __cplusplus
}  // extern "C"
#endif

#endif
