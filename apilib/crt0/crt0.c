#include "apilib.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

extern int main(int argc, char* argv[]);

/* These magic symbols are provided by the linker.  */
extern void (*__preinit_array_start []) (void) __attribute__((weak));
extern void (*__preinit_array_end []) (void) __attribute__((weak));
extern void (*__init_array_start []) (void) __attribute__((weak));
extern void (*__init_array_end []) (void) __attribute__((weak));
extern void (*__fini_array_start []) (void) __attribute__((weak));
extern void (*__fini_array_end []) (void) __attribute__((weak));

void HariMain(void) {
  api_initmalloc();

  char cmdline[256];
  api_cmdline(cmdline, sizeof(cmdline));  // TODO: Check command line argument length.

  int argc = 0;
  char* argv[16];

  char* p = cmdline;
  // TODO: Check argument count not to overflow.
  for (;;) {
    if (*p == '\0')
      break;
    argv[argc++] = p;
    p = p + strlen(p) + 1;
  }
  argv[argc] = NULL;

  for (void (**pp)() = __preinit_array_start; pp < __preinit_array_end; ++pp)
    (**pp)();
  // TODO: Init.
  for (void (**pp)() = __init_array_start; pp < __init_array_end; ++pp)
    (**pp)();

  int result = main(argc, argv);
  exit(result);
}
