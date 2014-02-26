#include "stdio.h"
#include "stdlib.h"

void HariMain(void) {
  char a[100];
  a[10] = 'A';  // ok
  putchar(a[10]);
  a[102] = 'B';  // ng
  putchar(a[102]);
  a[123] = 'C';  // ng
  putchar(a[123]);
  exit(0);
}
