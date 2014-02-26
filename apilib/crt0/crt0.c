#include "apilib.h"
#include "stdio.h"
#include "stdlib.h"

extern int main(int argc, char* argv[]);

void HariMain(void) {
  api_initmalloc();
  char* argv[] = { "foo", NULL };
  int result = main(1, argv);
  exit(result);
}
