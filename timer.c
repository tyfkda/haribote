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
  for (int i = 0; i < MAX_TIMER; ++i)
    timerctl.timers0[i].flags = 0;
  TIMER* t = timer_alloc();
  t->timeout = (unsigned int)-1;
  t->flags = TIMER_FLAGS_USING;
  t->next = NULL;
  timerctl.t0 = t;
  timerctl.next = (unsigned int)-1;
}

void inthandler20(int* esp) {
  (void)esp;
  io_out8(PIC0_OCW2, 0x60);  // Notify IRQ-00 recv to PIC
  ++timerctl.count;
  if (timerctl.next > timerctl.count)
    return;
  TIMER* timer = timerctl.t0;
  for (;;) {
    if (timer->timeout > timerctl.count)
      break;
    timer->flags = TIMER_FLAGS_ALLOC;
    fifo_put(timer->fifo, timer->data);
    timer = timer->next;
  }
  timerctl.t0 = timer;
  timerctl.next = timerctl.t0->timeout;
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

void timer_init(TIMER* timer, FIFO* fifo, int data) {
  timer->fifo = fifo;
  timer->data = data;
}

void timer_settime(TIMER* timer, unsigned int timeout) {
  timer->timeout = timeout + timerctl.count;
  timer->flags = TIMER_FLAGS_USING;
  int e = io_load_eflags();
  io_cli();
  TIMER* t = timerctl.t0, *s = NULL;
  if (timer->timeout <= t->timeout) {  // Earliest timer.
    timerctl.t0 = timer;
    timer->next = t;
    timerctl.next = timer->timeout;
  } else {
    for (;;) {
      s = t;
      t = t->next;
      if (timer->timeout <= t->timeout) {
        s->next = timer;
        timer->next = t;
        break;
      }
    }
  }
  io_store_eflags(e);
}
