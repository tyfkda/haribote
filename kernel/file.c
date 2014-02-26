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

void file_delete(FILEINFO* finfo, short* fat) {
  finfo->name[0] = 0xe5;  // Delete mark.
  for (short cluster = finfo->clustno; cluster < 0xff0; ) {
    short next = get_next_fat(fat, cluster);
    set_next_fat(fat, cluster, 0x000);  // Free
    cluster = next;
  }
}

static const unsigned char* clusterData(const void* diskImage, int cluster) {
  return (unsigned char*)diskImage + cluster * CLUSTER_SIZE;
}

int file_read(FILEHANDLE* fh, void* dst, int requestSize, const char* diskImage) {
  int readSize = 0;
  unsigned char* p = dst;
  while (requestSize > 0) {
    if (fh->pos >= (int)fh->finfo->size)
      break;
    char forward = TRUE;
    int nextClusterPos = (fh->pos + CLUSTER_SIZE) & -CLUSTER_SIZE;
    if (nextClusterPos > (int)fh->finfo->size)
      nextClusterPos = fh->finfo->size;
    int blockBytes = nextClusterPos - fh->pos;
    if (blockBytes > requestSize) {
      blockBytes = requestSize;
      forward = FALSE;
    }
    const unsigned char* src = clusterData(diskImage, fh->cluster) + (fh->pos % CLUSTER_SIZE);
    memcpy(p, src, blockBytes);
    p += blockBytes;
    fh->pos += blockBytes;
    if (forward)
      fh->cluster = get_next_fat(fh->fat, fh->cluster);
    readSize += blockBytes;
    requestSize -= blockBytes;
  }
  return readSize;
}

void file_loadfile(FILEINFO* finfo, const short* fat, char* img, void* buf) {
  FILEHANDLE fh;
  fh.finfo = finfo;
  fh.fat = fat;
  fh.pos = 0;
  fh.cluster = finfo->clustno;
  file_read(&fh, buf, finfo->size, img);
}

static int calc_cluster(FILEHANDLE* fh, int newpos) {
  int clusterCount = newpos / CLUSTER_SIZE - fh->pos / CLUSTER_SIZE;
  int cluster = fh->cluster;
  if (newpos < fh->pos) {  // If the new position is backward,
    // Then search target cluster from the top.
    cluster = fh->finfo->clustno;
    clusterCount = newpos / CLUSTER_SIZE;
  }

  for (int i = 0; i < clusterCount; ++i)
    cluster = get_next_fat(fh->fat, cluster);
  return cluster;
}

void file_seek(FILEHANDLE* fh, int offset, int origin) {
  int newpos = fh->pos;
  switch (origin) {
  case 0:  newpos = offset; break;
  case 1:  newpos += offset; break;
  case 2:  newpos = fh->finfo->size + offset; break;
  }
  if (newpos < 0)
    newpos = 0;
  else if (newpos > (int)fh->finfo->size)
    newpos = fh->finfo->size;
  fh->cluster = calc_cluster(fh, newpos);
  fh->pos = newpos;
}
