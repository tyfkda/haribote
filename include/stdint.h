#ifndef __STDINT_H__
#define __STDINT_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef signed char     int8_t;
typedef short           int16_t;
typedef int             int32_t;
typedef unsigned char   uint8_t;
typedef unsigned short  uint16_t;
typedef unsigned int    uint32_t;

typedef unsigned int size_t;

#define SIZE_MAX    (0xffffffffU)
#define UINT16_MAX  ((uint16_t)0x7fff)
#define INT32_MAX   ((int32_t)0x7fffffff)
#define INT32_MIN   ((int32_t)-0x80000000)

typedef long long           intmax_t;
typedef unsigned long long  uintmax_t;

typedef int             intptr_t;
typedef unsigned int    uintptr_t;

#ifdef __cplusplus
}  // extern "C"
#endif

#endif
