#include "stdio.h"
#include "api.h"

void exit(int status) {
  // TODO: Handle atexit.
  // TODO: Handle exit status.
  (void)status;
  api_end();
}
