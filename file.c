#include "file.h"
#include "stdio.h"  // memcpy

void file_readfat(short* fat, unsigned char* img) {
  for (int i = 0, j = 0; i < 2880; i += 2, j += 3) {
    fat[i + 0] = (img[j + 0]      | img[j + 1] << 8) & 0xfff;
    fat[i + 1] = (img[j + 1] >> 4 | img[j + 2] << 4) & 0xfff;
  }
}

void file_loadfile(short clustno, int size, char* buf, const short* fat, char* img) {
  for (;;) {
    if (size <= 512) {
      memcpy(buf, &img[clustno * 512], size);
      break;
    }
    memcpy(buf, &img[clustno * 512], 512);
    size -= 512;
    buf += 512;
    clustno = fat[clustno];
  }
}
