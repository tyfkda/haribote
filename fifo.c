#include "fifo.h"

const int FLAGS_OVERRUN = 1 << 0;

void fifo8_init(struct FIFO8* fifo, int size, unsigned char* buf) {
  fifo->buf = buf;
  fifo->size = fifo->free = size;
  fifo->p = fifo->q = 0;
  fifo->flags = 0;
}

int fifo8_put(struct FIFO8* fifo, unsigned char data) {
  if (fifo->free == 0) {
    fifo->flags |= FLAGS_OVERRUN;
    return -1;
  }
  fifo->buf[fifo->p] = data;
  if (++fifo->p >= fifo->size)
    fifo->p = 0;
  --fifo->free;
  return 0;
}

int fifo8_get(struct FIFO8* fifo) {
  if (fifo->free == fifo->size)  // Empty.
    return -1;
  int data = fifo->buf[fifo->q];
  if (++fifo->q >= fifo->size)
    fifo->q = 0;
  ++fifo->free;
  return data;
}

int fifo8_status(struct FIFO8* fifo) {
  return fifo->size - fifo->free;
}
