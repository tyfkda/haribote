#include "stdio.h"

int main(int argc, char* argv[]) {
  const char* format = "%s";
  for (int i = 1; i < argc; ++i) {
    printf(format, argv[i]);
    format = " %s";
  }
  putchar('\n');
  return 0;
}
