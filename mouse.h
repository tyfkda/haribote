// Mouse

#ifndef __MOUSE_H__
#define __MOUSE_H__

typedef struct {
  unsigned char buf[3], phase;
  int x, y, btn;
} MOUSE_DEC;

extern struct FIFO8 mousefifo;

void enable_mouse(MOUSE_DEC* mdec);
int mouse_decode(MOUSE_DEC* mdec, unsigned dat);

#endif
