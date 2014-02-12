#ifndef __FILE_H__
#define __FILE_H__

typedef struct {
  unsigned char name[8], ext[3], type, reserve[10];
  unsigned short time, date, clustno;
  unsigned int size;
} FILEINFO;

void file_readfat(short* fat, unsigned char* img);
FILEINFO* file_search(const char* name, FILEINFO* finfo, int max);
void file_loadfile(short clustno, int size, char* buf, const short* fat, char* img);

#endif
