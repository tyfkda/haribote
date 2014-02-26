#ifndef __FILE_H__
#define __FILE_H__

typedef struct {
  unsigned char name[8], ext[3], type, reserve[10];
  unsigned short time, date, clustno;
  unsigned int size;
} FILEINFO;

typedef struct FILEHANDLE {
  FILEINFO* finfo;
  const short* fat;
  int pos;
  short cluster;
} FILEHANDLE;

void file_readfat(short* fat, unsigned char* img);
FILEINFO* file_search(const char* name, FILEINFO* finfo, int max);
void file_loadfile(FILEINFO* finfo, const short* fat, char* img, void* buf);
void file_delete(FILEINFO* finfo, short* fat);
int file_read(FILEHANDLE* fh, void* dst, int requestSize, const char* diskImage);
void file_seek(FILEHANDLE* fh, int offset, int origin);

#endif
