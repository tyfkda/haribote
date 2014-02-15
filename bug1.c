#include "api.h"

void HariMain(void) {
  char a[100];
  a[10] = 'A';  // ok
  api_putchar(a[10]);
  a[102] = 'B';  // ng
  api_putchar(a[102]);
  a[123] = 'C';  // ng
  api_putchar(a[123]);
  api_end();
}
