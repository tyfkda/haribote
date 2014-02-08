// FIFO

#ifndef __FIFO_H__
#define __FIFO_H__

typedef struct FIFO8 {
  unsigned char* buf;
  int p, q, size, free, flags;
} FIFO8;

void fifo8_init(FIFO8* fifo, int size, unsigned char* buf);
int fifo8_put(FIFO8* fifo, unsigned char data);
int fifo8_get(FIFO8* fifo);
int fifo8_status(FIFO8* fifo);

#endif
