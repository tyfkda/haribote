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
    if (timer != task_timer) {
      fifo_put(timer->fifo, timer->data);
    } else {
      ts = 1;
    }
    timer = timer->next_timer;
  }
  timerctl.t0 = timer;
  timerctl.next_time = timerctl.t0->timeout;

  if (ts)
    task_switch();
}

TIMER* timer_alloc(void) {
  for (int i = 0; i < MAX_TIMER; ++i) {
    TIMER* timer = &timerctl.timers0[i];
    if (timer->flags == 0) {
      timer->flags = TIMER_FLAGS_ALLOC;
      timer->flags2 = 0;
      return timer;
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

int timer_cancel(TIMER* timer) {
  int e = io_load_eflags();
  io_cli();
  if (timer->flags == TIMER_FLAGS_USING) {
    if (timer == timerctl.t0) {
      TIMER* t = timer->next_timer;
      timerctl.t0 = t;
      timerctl.next_time = t->timeout;
    } else {
      TIMER* t = timerctl.t0;
      for (;; t = t->next_timer) {
        if (t->next_timer == timer)
          break;
      }
      t->next_timer = timer->next_timer;
    }
    timer->flags = TIMER_FLAGS_ALLOC;
    io_store_eflags(e);
    return TRUE;
  }
  io_store_eflags(e);
  return FALSE;
}

void timer_cancelall(FIFO* fifo) {
  int e = io_load_eflags();
  io_cli();
  for (int i = 0; i < MAX_TIMER; ++i) {
    TIMER* t = &timerctl.timers0[i];
    if (t->flags != 0 && t->flags2 != 0 && t->fifo == fifo) {
      timer_cancel(t);
      timer_free(t);
    }
  }
  io_store_eflags(e);
}
