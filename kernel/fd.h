// Handling Floppy Disk image.

#ifndef __FILE_H__
#define __FILE_H__

typedef struct {
  unsigned char name[8], ext[3], type, reserve[10];
  unsigned short time, date, clustno;
  unsigned int size;
} FDINFO;

typedef struct FDHANDLE {
  FDINFO* finfo;
  int pos;
  short cluster;
  char modified;
} FDHANDLE;

int fd_open(FDHANDLE* fh, const char* name);
int fd_writeopen(FDHANDLE* fh, const char* filename);
int fd_read(FDHANDLE* fh, void* dst, int requestSize);
int fd_write(FDHANDLE* fh, const void* src, int requestSize);
void fd_seek(FDHANDLE* fh, int offset, int origin);
void fd_close(FDHANDLE* fh);
int fd_delete(const char* filename);

short get_next_cluster(short cluster);

void init_fdc();
char* fdc_read(int head, int track, int sector, int length);

#endif
