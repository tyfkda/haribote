// Interrupt

#include "int.h"
#include "bootpack.h"
#include "fifo.h"
#include "graphic.h"
#include "stdio.h"

static const int PORT_KEYDAT = 0x0060;

struct FIFO8 keyfifo;
struct FIFO8 mousefifo;

void init_pic(void) {
  io_out8(PIC0_IMR, 0xff);  // Prevent all interrupt.
  io_out8(PIC1_IMR, 0xff);

  io_out8(PIC0_ICW1, 0x11);    // Edge trigger mode.
  io_out8(PIC0_ICW2, 0x20);    // IRQ0-7 => INT20-27
  io_out8(PIC0_ICW3, 1 << 2);  // PIC1 => IRQ2
  io_out8(PIC0_ICW4, 0x01);    // Non buffer mode.

  io_out8(PIC1_ICW1, 0x11);    // Edge trigger mode.
  io_out8(PIC1_ICW2, 0x28);    // IRQ8-15 => INT28-2f
  io_out8(PIC1_ICW3, 2);       // PIC1 => IRQ2
  io_out8(PIC1_ICW4, 0x01);    // Non buffer mode.

  io_out8(PIC0_IMR, 0xfb);     // Enable PIC1
  io_out8(PIC1_IMR, 0xff);
}

void inthandler21(int* esp) {
  (void)esp;
  io_out8(PIC0_OCW2, 0x61);  // Notify IRQ-01 recv finish to PIC
  unsigned char data = io_in8(PORT_KEYDAT);
  fifo8_put(&keyfifo, data);
}

void inthandler2c(int* esp) {
  (void)esp;
  io_out8(PIC1_OCW2, 0x64);  // Notify IRQ-12 recv finish to PIC1
  io_out8(PIC0_OCW2, 0x62);  // Notify IRQ-02 recv finish to PIC0
  unsigned char data = io_in8(PORT_KEYDAT);
  fifo8_put(&mousefifo, data);
}
