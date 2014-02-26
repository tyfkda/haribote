#include "stdio.h"
#include "apilib.h"

void exit(int status) {
  // TODO: Handle atexit.
  // TODO: Handle exit status.
  (void)status;
  api_end();
}
