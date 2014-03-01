#ifndef __FILE_H__
#define __FILE_H__

typedef struct {
  unsigned char name[8], ext[3], type, reserve[10];
  unsigned short time, date, clustno;
  unsigned int size;
} FILEINFO;

typedef struct FILEHANDLE {
  FILEINFO* finfo;
  int pos;
  short cluster;
  char modified;
} FILEHANDLE;

FILEINFO* file_search(const char* name);
FILEINFO* file_create(const char* name);
void file_loadfile(FILEINFO* finfo, void* buf);
void file_delete(FILEINFO* finfo);
int file_read(FILEHANDLE* fh, void* dst, int requestSize);
int file_write(FILEHANDLE* fh, const void* src, int requestSize);
void file_seek(FILEHANDLE* fh, int offset, int origin);
void file_close(FILEHANDLE* fh);

#endif
