// Mouse

#ifndef __MOUSE_H__
#define __MOUSE_H__

struct MOUSE_DEC {
  unsigned char buf[3], phase;
  int x, y, btn;
};

extern struct FIFO8 mousefifo;

void enable_mouse(struct MOUSE_DEC* mdec);
int mouse_decode(struct MOUSE_DEC* mdec, unsigned dat);

#endif
