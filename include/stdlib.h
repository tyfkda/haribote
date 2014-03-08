#ifndef __STDLIB_H__
#define __STDLIB_H__

#include "stdint.h"

#ifdef __cplusplus
extern "C" {
#endif

#define EXIT_SUCCESS  (0)
#define EXIT_FAILURE  (1)

#define RAND_MAX  (0x7fff)

void exit(int status) __attribute__((__noreturn__));
int atexit(void (*function)(void));

void* malloc(size_t size);
void free(void* p);

long strtol(const char *s, char **endp, int base);

int rand(void);

void abort(void);
int atoi(const char *nptr);
unsigned long int strtoul(const char *nptr, char **endptr, int base);
double strtod(const char *nptr, char **endptr);
int abs(int j);
void* realloc(void* ptr, size_t size);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif
