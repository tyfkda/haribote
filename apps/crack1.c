#include "stdlib.h"

void HariMain(void) {
  *((char*)0x00102600) = 0;
  exit(0);
}
