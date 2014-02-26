static int rand_x;

int rand() {
  int a = 1103515245, b = 12345, c = 2147483647;
  rand_x = (a * rand_x + b) & c;
  return (rand_x >> 16) & 0x7fff;  // RAND_MAX
}
