#include "stdio.h"

char buf[256];

void write1() {
  int i;
  for (i = 0; i <= 'z' - '@'; ++i)
    buf[i] = i + '@';
  FILE* fp = fopen("test.txt", "w");
  if (fp == NULL) {
    printf("Cannot open file\n");
    return;
  }

  printf("FilePointer: %p\n", fp);
  fwrite(buf, 1, i, fp);

  fprintf(fp, "\nhoge:%d\n", 123);

  fclose(fp);
}

void write2() {
  fprintf(stdout, "fuga:%d\n", 456);
}

int main() {
  write1();
  write2();
  return 0;
}
