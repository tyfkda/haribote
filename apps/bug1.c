#include "stdio.h"

int main() {
  char a[100];
  a[10] = 'A';  // ok
  putchar(a[10]);
  a[102] = 'B';  // ng
  putchar(a[102]);
  a[123] = 'C';  // ng
  putchar(a[123]);
  return 0;
}
