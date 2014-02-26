#ifndef __FILE_H__
#define __FILE_H__

typedef struct FILEHANDLE {
  unsigned char* buf;
  int size;
  int pos;
} FILEHANDLE;

typedef struct {
  unsigned char name[8], ext[3], type, reserve[10];
  unsigned short time, date, clustno;
  unsigned int size;
} FILEINFO;

void file_readfat(short* fat, unsigned char* img);
FILEINFO* file_search(const char* name, FILEINFO* finfo, int max);
void file_loadfile(short clustno, int size, void* buf, const short* fat, char* img);
void file_delete(FILEINFO* finfo, short* fat);

#endif
