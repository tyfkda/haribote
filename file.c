#include "file.h"
#include "stdio.h"  // memcpy

void file_readfat(short* fat, unsigned char* img) {
  for (int i = 0, j = 0; i < 2880; i += 2, j += 3) {
    fat[i + 0] = (img[j + 0]      | img[j + 1] << 8) & 0xfff;
    fat[i + 1] = (img[j + 1] >> 4 | img[j + 2] << 4) & 0xfff;
  }
}

FILEINFO* file_search(const char* name, FILEINFO* finfo, int max) {
  char s[12];
  memset(s, ' ', 11);
  int j = 0;
  for (int i = 0; j < 11 && name[i] != '\0'; ++i) {
    if (name[i] == '.') {
      j = 8;
    } else {
      s[j++] = toupper(name[i]);
    }
  }
  for (int i = 0; i < max; ++i, ++finfo) {
    if (finfo->name[0] == 0x00)  // End of table.
      return NULL;
    if ((finfo->type & 0x18) == 0) {
      if (strncmp((char*)finfo->name, s, 11) == 0)
        return finfo;
    }
  }
  return NULL;
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
