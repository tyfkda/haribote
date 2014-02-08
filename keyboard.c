// Keyboard

#include "keyboard.h"
#include "bootpack.h"
#include "fifo.h"
#include "int.h"

static const int PORT_KEYSTA = 0x0064;
static const int KEYSTA_SEND_NOTREADY = 0x02;
static const int KEYCMD_WRITE_MODE = 0x60;
static const int KBC_MODE = 0x47;

struct FIFO8 keyfifo;

void inthandler21(int* esp) {
  (void)esp;
  io_out8(PIC0_OCW2, 0x61);  // Notify IRQ-01 recv finish to PIC
  unsigned char data = io_in8(PORT_KEYDAT);
  fifo8_put(&keyfifo, data);
}

void wait_KBC_sendready(void) {
  for (;;)
    if ((io_in8(PORT_KEYSTA) & KEYSTA_SEND_NOTREADY) == 0)
      break;
}

void init_keyboard(void) {
  wait_KBC_sendready();
  io_out8(PORT_KEYCMD, KEYCMD_WRITE_MODE);
  wait_KBC_sendready();
  io_out8(PORT_KEYDAT, KBC_MODE);
}
