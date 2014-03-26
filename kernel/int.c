// Interrupt

#include "int.h"
#include "naskfunc.h"

void init_pic(void) {
  io_out8(PIC0_IMR, 0xff);     // Prevent all interrupt.
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
