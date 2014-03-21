#include "stdio.h"

int main(int argc, char* argv[]) {
  for (int i = 1; i < argc; ++i) {
    const char* filename = argv[i];
    FILE* fp = fopen(filename, "r");
    if (fp == NULL) {
      printf("File not found: %s\n", filename);
      return 1;
    }
    for (;;) {
      int c = fgetc(fp);
      if (c == EOF)
        break;
      putchar(c);
    }
    fclose(fp);
  }
  return 0;
}
