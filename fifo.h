// FIFO

#ifndef __FIFO_H__
#define __FIFO_H__

typedef struct FIFO {
  int* buf;
  int p, q, size, free, flags;
} FIFO;

void fifo_init(FIFO* fifo, int size, int* buf);
int fifo_put(FIFO* fifo, int data);
int fifo_get(FIFO* fifo);
int fifo_status(FIFO* fifo);

#endif
