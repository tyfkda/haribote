#include "file.h"
#include "bootpack.h"
#include "stdio.h"  // NULL
#include "string.h"  // memcpy, strncmp

#define MAX_CLUSTER  (2880)
#define CLUSTER_SIZE  (512)

#define FINFO_TOP  ((FILEINFO*)(ADR_DISKIMG + 0x002600))
#define FINFO_MAX  (224)

static short get_next_cluster(const short* fat, short cluster) {
  return fat[cluster];
}

static void set_next_cluster(short* fat, short cluster, short next) {
  fat[cluster] = next;
}

void file_close(FILEHANDLE* fh) {
  if (fh->modified) {
    if (fh->cluster > 0)
      set_next_cluster(fh->fat, fh->cluster, 0xfff);  // End mark.
    fh->finfo->size = fh->pos;
  }
  fh->finfo = NULL;
}

void file_readfat(short* fat, unsigned char* img) {
  for (int i = 0, j = 0; i < MAX_CLUSTER; i += 2, j += 3) {
    fat[i + 0] = (img[j + 0]      | img[j + 1] << 8) & 0xfff;
    fat[i + 1] = (img[j + 1] >> 4 | img[j + 2] << 4) & 0xfff;
  }
}

static void make_file_name83(char s[12], const char* name) {
  memset(s, ' ', 11);
  int j = 0;
  for (int i = 0; j < 11 && name[i] != '\0'; ++i) {
    if (name[i] == '.') {
      j = 8;
    } else {
      s[j++] = name[i];
    }
  }
}

FILEINFO* file_create(const char* name) {
  char s[12];
  make_file_name83(s, name);
  for (int i = 0; i < FINFO_MAX; ++i) {
    FILEINFO* finfo = &FINFO_TOP[i];
    if (finfo->name[0] == 0x00 || finfo->name[0] == 0xe5) {  // End of table, or deleted entry.
      memset(finfo, 0x00, sizeof(FILEINFO));
      memcpy(finfo->name, s, 11);
      finfo->type = 0x20;  // Normal file.
      return finfo;
    }
    if ((finfo->type & 0x18) == 0) {
      if (strncmp((char*)finfo->name, s, 11) == 0)
        return finfo;
    }
  }
  return NULL;
}

FILEINFO* file_search(const char* name) {
  char s[12];
  make_file_name83(s, name);
  for (int i = 0; i < FINFO_MAX; ++i) {
    FILEINFO* finfo = &FINFO_TOP[i];
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
    short next = get_next_cluster(fat, cluster);
    set_next_cluster(fat, cluster, 0x000);  // Free
    cluster = next;
  }
}

static unsigned char* clusterData(const void* diskImage, int cluster) {
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
      fh->cluster = get_next_cluster(fh->fat, fh->cluster);
    readSize += blockBytes;
    requestSize -= blockBytes;
  }
  return readSize;
}

short allocate_cluster(short* fat) {
  for (int i = 0; i < MAX_CLUSTER; ++i) {
    if (fat[i] == 0) {
      fat[i] = 0xfff;  // Write end mark.
      return i;
    }
  }
  return -1;
}

int file_write(FILEHANDLE* fh, const void* srcData, int requestSize, const char* diskImage) {
  if (requestSize <= 0)
    return 0;

  fh->modified = TRUE;
  int writeSize = 0;
  unsigned char* src = (unsigned char*)srcData;
  while (requestSize > 0) {
    if (fh->pos == 0) {  // First write.
      if (fh->finfo->clustno > 0) {  // Exist old file: overwrite.
        fh->cluster = fh->finfo->clustno;
      } else {  // Not exist: allocate new cluster.
        fh->cluster = fh->finfo->clustno = allocate_cluster(fh->fat);
        // TODO: Error check.
      }
    } else if ((fh->pos % CLUSTER_SIZE) == 0) {  // Forward next cluster.
      short nextCluster = get_next_cluster(fh->fat, fh->cluster);
      if (nextCluster < 0xff0) {  // Valid: use it.
        fh->cluster = nextCluster;
      } else {
        nextCluster = allocate_cluster(fh->fat);
        // TODO: Error check.
        set_next_cluster(fh->fat, fh->cluster, nextCluster);
        fh->cluster = nextCluster;
      }
    }

    int size = CLUSTER_SIZE - (fh->pos % CLUSTER_SIZE);
    if (requestSize < size)
      size = requestSize;
    unsigned char* dst = clusterData(diskImage, fh->cluster) + (fh->pos % CLUSTER_SIZE);
    memcpy(dst, src, size);
    fh->pos += size;
    writeSize += size;
    dst += size;
    requestSize -= size;
  }
  return writeSize;
}

void file_loadfile(FILEINFO* finfo, short* fat, char* img, void* buf) {
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
    cluster = get_next_cluster(fh->fat, cluster);
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
