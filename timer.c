#include "timer.h"
#include "bootpack.h"
#include "int.h"

#define PIT_CTRL  (0x0043)
#define PIT_CNT0  (0x0040)

TIMERCTL timerctl;

void init_pit(void) {
  io_out8(PIT_CTRL, 0x34);
  io_out8(PIT_CNT0, 0x9c);
  io_out8(PIT_CNT0, 0x2e);
  timerctl.count = 0;
}

void inthandler20(int* esp) {
  (void)esp;
  io_out8(PIC0_OCW2, 0x60);  // Notify IRQ-00 recv to PIC
  ++timerctl.count;
}
