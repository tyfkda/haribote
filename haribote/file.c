#include "file.h"
#include "stdio.h"  // NULL
#include "string.h"  // memcpy, strncmp

#define MAX_CLUSTER  (2880)
#define CLUSTER_SIZE  (512)

static short get_next_fat(const short* fat, short cluster) {
  return fat[cluster];
}

static void set_next_fat(short* fat, short cluster, short next) {
  fat[cluster] = next;
}

void file_readfat(short* fat, unsigned char* img) {
  for (int i = 0, j = 0; i < MAX_CLUSTER; i += 2, j += 3) {
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
      s[j++] = name[i];
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

void file_loadfile(short clustno, int size, void* buf, const short* fat, char* img) {
  char* p = buf;
  for (;;) {
    if (size <= CLUSTER_SIZE) {
      memcpy(p, &img[clustno * CLUSTER_SIZE], size);
      break;
    }
    memcpy(p, &img[clustno * CLUSTER_SIZE], CLUSTER_SIZE);
    size -= CLUSTER_SIZE;
    p += CLUSTER_SIZE;
    clustno = fat[clustno];
  }
}

void file_delete(FILEINFO* finfo, short* fat) {
  finfo->name[0] = 0xe5;  // Delete mark.
  for (short cluster = finfo->clustno; cluster < 0xff0; ) {
    short next = get_next_fat(fat, cluster);
    set_next_fat(fat, cluster, 0x000);  // Free
    cluster = next;
  }
}
