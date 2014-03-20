#include "stdio.h"
#include "stdlib.h"

int main(int argc, char* argv[]) {
  const char* fn = "foo";
  int len = 256;
  if (argc >= 2) {
    fn = argv[1];
    if (argc >= 3) {
      len = atoi(argv[2]);
    }
  }

  FILE* fp = fopen(fn, "wb");
  if (fp == NULL) {
    fprintf(stderr, "Cannot open file: %s\n", fn);
    return 1;
  }

  char buf[256];
  for (int i = 0; i < 256; ++i)
    buf[i] = i;

  for (; len >= 256; len -= 256)
    fwrite(buf, 1, 256, fp);
  if (len > 0)
    fwrite(buf, 1, len, fp);

  fclose(fp);
  return 0;
}
