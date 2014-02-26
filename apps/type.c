#include "apilib.h"
#include "stdio.h"

int main(int argc, char* argv[]) {
  for (int i = 1; i < argc; ++i) {
    const char* filename = argv[i];
    int fh = api_fopen(filename, OPEN_READ);
    if (fh == 0) {
      printf("File not found: %s\n", filename);
      return 1;
    }
    for (;;) {
      char c;
      if (api_fread(&c, 1, fh) == 0)
        break;
      putchar(c);
    }
  }
  return 0;
}
