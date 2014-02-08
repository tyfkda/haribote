#include "timer.h"
#include "bootpack.h"
#include "int.h"
#include "mtask.h"
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
  timerctl.next_time = (unsigned int)-1;
  for (int i = 0; i < MAX_TIMER; ++i)
    timerctl.timers0[i].flags = 0;
  TIMER* t = timer_alloc();
  t->timeout = (unsigned int)-1;
  t->flags = TIMER_FLAGS_USING;
  t->next_timer = NULL;
  timerctl.t0 = t;
  timerctl.next_time = (unsigned int)-1;
}

void inthandler20(int* esp) {
  (void)esp;
  char ts = 0;  // Task switch?
  io_out8(PIC0_OCW2, 0x60);  // Notify IRQ-00 recv to PIC
  ++timerctl.count;
  if (timerctl.next_time > timerctl.count)
    return;
  TIMER* timer = timerctl.t0;
  for (;;) {
    if (timer->timeout > timerctl.count)
      break;
    timer->flags = TIMER_FLAGS_ALLOC;
    if (timer != mt_timer) {
      fifo_put(timer->fifo, timer->data);
    } else {
      ts = 1;
    }
    timer = timer->next_timer;
  }
  timerctl.t0 = timer;
  timerctl.next_time = timerctl.t0->timeout;

  if (ts) {
    mt_taskswitch();
  }
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
    timer->next_timer = t;
    timerctl.next_time = timer->timeout;
  } else {
    for (;;) {
      s = t;
      t = t->next_timer;
      if (timer->timeout <= t->timeout) {
        s->next_timer = timer;
        timer->next_timer = t;
        break;
      }
    }
  }
  io_store_eflags(e);
}
