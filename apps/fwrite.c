#include "apilib.h"
#include "stdio.h"

char buf[256];

int main() {
  int i;
  for (i = 0; i <= 'z' - '@'; ++i)
    buf[i] = i + '@';
  int fh = api_fopen("test.txt", OPEN_WRITE);
  printf("FileHandle: %d\n", fh);
  if (fh != 0) {
    api_fwrite(buf, i, fh);
    api_fclose(fh);
  }
  return 0;
}
