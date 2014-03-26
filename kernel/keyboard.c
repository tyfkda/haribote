// Keyboard

#include "keyboard.h"
#include "fifo.h"
#include "int.h"
#include "naskfunc.h"

#define PORT_KEYSTA  (0x0064)
#define KEYSTA_SEND_NOTREADY  (0x02)
#define KEYCMD_WRITE_MODE  (0x60)
#define KBC_MODE  (0x47)

static FIFO* keyfifo;
static int keydata0;

// IRQ-21 : Keyboard interrupt.
void inthandler21(int* esp) {
  (void)esp;
  io_out8(PIC0_OCW2, 0x61);  // Notify IRQ-01 recv finish to PIC
  unsigned char data = io_in8(PORT_KEYDAT);
  fifo_put(keyfifo, data + keydata0);
}

void wait_KBC_sendready(void) {
  for (;;)
    if ((io_in8(PORT_KEYSTA) & KEYSTA_SEND_NOTREADY) == 0)
      break;
}

void init_keyboard(FIFO* fifo, int data0) {
  keyfifo = fifo;
  keydata0 = data0;

  wait_KBC_sendready();
  io_out8(PORT_KEYCMD, KEYCMD_WRITE_MODE);
  wait_KBC_sendready();
  io_out8(PORT_KEYDAT, KBC_MODE);
}
