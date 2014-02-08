#include "timer.h"
#include "bootpack.h"
#include "int.h"
#include "stdio.h"  // NULL

#define PIT_CTRL  (0x0043)
#define PIT_CNT0  (0x0040)

#define TIMER_FLAGS_ALLOC  (1)
#define TIMER_FLAGS_USING  (2)

TIMERCTL timerctl;

void init_pit(void) {
  io_out8(PIT_CTRL, 0x34);
  io_out8(PIT_CNT0, 0x9c);
  io_out8(PIT_CNT0, 0x2e);
  timerctl.count = 0;
  timerctl.next = (unsigned int)-1;
  timerctl.using = 0;
  for (int i = 0; i < MAX_TIMER; ++i)
    timerctl.timers0[i].flags = 0;
}

void inthandler20(int* esp) {
  (void)esp;
  io_out8(PIC0_OCW2, 0x60);  // Notify IRQ-00 recv to PIC
  ++timerctl.count;
  if (timerctl.next > timerctl.count)
    return;
  unsigned int i;
  for (i = 0; i < timerctl.using; ++i) {
    TIMER* timer = timerctl.timers[i];
    if (timer->timeout > timerctl.count)
      break;
    timer->flags = TIMER_FLAGS_ALLOC;
    fifo8_put(timer->fifo, timer->data);
  }
  timerctl.using -= i;
  for (unsigned int j = 0; j < timerctl.using; ++j)
    timerctl.timers[j] = timerctl.timers[j + i];
  if (timerctl.using > 0)
    timerctl.next = timerctl.timers[0]->timeout;
  else
    timerctl.next = (unsigned int)-1;
}

TIMER* timer_alloc(void) {
  for (int i = 0; i < MAX_TIMER; ++i) {
    if (timerctl.timers0[i].flags == 0) {
      timerctl.timers0[i].flags = TIMER_FLAGS_ALLOC;
      return &timerctl.timers0[i];
    }
  }
  return NULL;
}

void timer_free(TIMER* timer) {
  timer->flags = 0;
}

void timer_init(TIMER* timer, FIFO8* fifo, unsigned char data) {
  timer->fifo = fifo;
  timer->data = data;
}

void timer_settime(TIMER* timer, unsigned int timeout) {
  timer->timeout = timeout + timerctl.count;
  timer->flags = TIMER_FLAGS_USING;
  int e = io_load_eflags();
  io_cli();
  unsigned int i;
  for (i = 0; i < timerctl.using; ++i) {
    if (timerctl.timers[i]->timeout >= timer->timeout)
      break;
  }
  ++timerctl.using;
  for (unsigned int j = timerctl.using; j > i; --j)
    timerctl.timers[j] = timerctl.timers[j - 1];
  timerctl.timers[i] = timer;
  timerctl.next = timerctl.timers[0]->timeout;
  io_store_eflags(e);
}
