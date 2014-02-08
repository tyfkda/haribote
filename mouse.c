// Mouse

#include "mouse.h"
#include "bootpack.h"
#include "fifo.h"
#include "int.h"
#include "keyboard.h"

static const int KEYCMD_SENDTO_MOUSE = 0xd4;
static const int MOUSECMD_ENABLE = 0xf4;

struct FIFO8 mousefifo;

void inthandler2c(int* esp) {
  (void)esp;
  io_out8(PIC1_OCW2, 0x64);  // Notify IRQ-12 recv finish to PIC1
  io_out8(PIC0_OCW2, 0x62);  // Notify IRQ-02 recv finish to PIC0
  unsigned char data = io_in8(PORT_KEYDAT);
  fifo8_put(&mousefifo, data);
}

void enable_mouse(struct MOUSE_DEC* mdec) {
  wait_KBC_sendready();
  io_out8(PORT_KEYCMD, KEYCMD_SENDTO_MOUSE);
  wait_KBC_sendready();
  io_out8(PORT_KEYDAT, MOUSECMD_ENABLE);
  // ACK(0xfa) will be sent.
  mdec->phase = 0;
}

int mouse_decode(struct MOUSE_DEC* mdec, unsigned dat) {
  switch (mdec->phase) {
  case 0:  // Waiting 0xfa.
    if (dat == 0xfa)
      mdec->phase = 1;
    return 0;
  case 1:  // Waiting first byte.
    if ((dat & 0xc8) == 0x08) {
      mdec->buf[0] = dat;
      mdec->phase = 2;
    }
    return 0;
  case 2:  // Waiting second byte.
    mdec->buf[1] = dat;
    mdec->phase = 3;
    return 0;
  case 3:  // Waiting thrid byte.
    mdec->buf[2] = dat;
    mdec->phase = 1;
    mdec->btn = mdec->buf[0] & 0x07;
    mdec->x = mdec->buf[1];
    mdec->y = mdec->buf[2];
    if ((mdec->buf[0] & 0x10) != 0)
      mdec->x |= -1 << 8;
    if ((mdec->buf[0] & 0x20) != 0)
      mdec->y |= -1 << 8;
    mdec->y = -mdec->y;
    return 1;
  }
  return -1;
}
