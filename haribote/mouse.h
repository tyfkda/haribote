// Mouse

#ifndef __MOUSE_H__
#define __MOUSE_H__

#include "fifo.h"

typedef struct {
  unsigned char buf[3], phase;
  int x, y, btn;
} MOUSE_DEC;

void enable_mouse(FIFO* fifo, int data0, MOUSE_DEC* mdec);
int mouse_decode(MOUSE_DEC* mdec, unsigned dat);

#endif
