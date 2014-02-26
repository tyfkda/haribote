#include "atexit.h"
#include "stdlib.h"

int atexit(void (*function)(void)) {
  if (_atexit.count >= ATEXIT_MAX)
    return 1;
  for (int i = 0; i < _atexit.count; ++i)
    if (_atexit.functions[i] == function)
      return 0;
  _atexit.functions[_atexit.count++] = function;
  return 0;
}
