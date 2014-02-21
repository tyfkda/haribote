#include "api.h"

void HariMain(void) {
  //api_putchar('A');
  *((char*)0x00102600) = 0;
  api_end();
}
