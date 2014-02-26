#include "stdlib.h"
#include "apilib.h"
#include "atexit.h"

AtExit _atexit;

void exit(int status) {
  for (int i = 0; i < _atexit.count; ++i) {
    ExitFunction f = _atexit.functions[i];
    (*f)();
  }

  // TODO: Handle exit status.
  (void)status;
  api_end();
}
