#include "util.h"
#include "bootpack.h"
#include "stddef.h"

static int bcd2(unsigned char x) {
  return (x >> 4) * 10 + (x & 0x0f);
}

int read_rtc(unsigned char tt[5]) {
  static const unsigned char adr[7] = { 0x00, 0x02, 0x04, 0x07, 0x08, 0x09, 0x32 };
  static const unsigned char max[7] = { 0x60, 0x59, 0x23, 0x31, 0x12, 0x99, 0x99 };
  unsigned char t[7];
  for (int i = 0; i < 7; i++) {
    io_out8(0x70, adr[i]);
    unsigned char v = io_in8(0x71);
    if (!((v & 0x0f) <= 9 && v <= max[i]))
      v = -1;
    t[i] = v;
  }
  char err = FALSE;
  do {
    for (int i = 0; i < 7; i++) {
      io_out8(0x70, adr[i]);
      unsigned char v = io_in8(0x71);
      if (t[i] != v) {
        if ((v & 0x0f) <= 9 && v <= max[i])
          t[i] = v;
        err = TRUE;
      }
    }
  } while (err);

  int year = bcd2(t[6]) * 100 + bcd2(t[5]);
  tt[0] = bcd2(t[4]);  // month.
  tt[1] = bcd2(t[3]);  // day.
  tt[2] = bcd2(t[2]);  // hour.
  tt[3] = bcd2(t[1]);  // minute.
  tt[4] = bcd2(t[0]);  // second.
  return year;
}
