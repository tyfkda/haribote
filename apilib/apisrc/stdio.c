#include "stdio_def.h"

static FILE _stdin = {
  kSTDIN,
};

static FILE _stdout = {
  kSTDOUT,
};

static FILE _stderr = {
  kSTDERR,
};


FILE *stdin = &_stdin;
FILE *stdout = &_stdout;
FILE *stderr = &_stderr;
