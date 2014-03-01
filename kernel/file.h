#ifndef __FILE_H__
#define __FILE_H__

typedef struct {
  unsigned char name[8], ext[3], type, reserve[10];
  unsigned short time, date, clustno;
  unsigned int size;
} FILEINFO;

typedef struct FILEHANDLE {
  FILEINFO* finfo;
  short* fat;
  int pos;
  short cluster;
  char modified;
} FILEHANDLE;

void file_readfat(short* fat, unsigned char* img);
FILEINFO* file_search(const char* name);
FILEINFO* file_create(const char* name);
void file_loadfile(FILEINFO* finfo, short* fat, char* img, void* buf);
void file_delete(FILEINFO* finfo, short* fat);
int file_read(FILEHANDLE* fh, void* dst, int requestSize, const char* diskImage);
int file_write(FILEHANDLE* fh, const void* src, int requestSize, const char* diskImage);
void file_seek(FILEHANDLE* fh, int offset, int origin);
void file_close(FILEHANDLE* fh);

#endif
