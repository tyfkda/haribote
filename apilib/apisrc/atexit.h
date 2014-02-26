#ifndef __ATEXIT_H__
#define __ATEXIT_H__

#define ATEXIT_MAX  (32)

typedef void (*ExitFunction)(void);

typedef struct {
  ExitFunction functions[ATEXIT_MAX];
  int count;
} AtExit;

extern AtExit _atexit;

#endif
