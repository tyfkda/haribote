#include "apilib.h"
#include "stdio.h"

void HariMain() {
  unsigned char now[7];
  api_now(now);
  int year = (now[0] << 8) | now[1];
  int month = now[2];
  int day = now[3];
  int hour = now[4];
  int minute = now[5];
  int second = now[6];
  printf("%04d/%02d/%02d %02d:%02d:%02d\n", year, month, day, hour, minute, second);
  exit(0);
}
