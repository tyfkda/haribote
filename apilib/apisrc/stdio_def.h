#ifndef __STDIO_DEF_H__
#define __STDIO_DEF_H__

#include "stdio.h"

#ifdef __cplusplus
extern "C" {
#endif

#define kSTDIN   (1)
#define kSTDOUT  (2)
#define kSTDERR  (3)

struct FILE {
  int handle;
};

#ifdef __cplusplus
}  // extern "C"
#endif

#endif
