#include "fifo.h"
#include "mtask.h"
#include "stdio.h"  // NULL

#define FLAGS_OVERRUN  (1 << 0)

void fifo_init(FIFO* fifo, int size, int* buf, TASK* task) {
  fifo->buf = buf;
  fifo->size = fifo->free = size;
  fifo->p = fifo->q = 0;
  fifo->flags = 0;
  fifo->task = task;
}

int fifo_put(FIFO* fifo, int data) {
  if (fifo->free == 0) {
    fifo->flags |= FLAGS_OVERRUN;
    return -1;
  }
  fifo->buf[fifo->p] = data;
  if (++fifo->p >= fifo->size)
    fifo->p = 0;
  --fifo->free;
  if (fifo->task != NULL)
    task_wake(fifo->task);
  return 0;
}

int fifo_get(FIFO* fifo) {
  if (fifo->free == fifo->size)  // Empty.
    return -1;
  int data = fifo->buf[fifo->q];
  if (++fifo->q >= fifo->size)
    fifo->q = 0;
  ++fifo->free;
  return data;
}

int fifo_empty(FIFO* fifo) {
  return fifo->size == fifo->free;
}
