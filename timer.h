#ifndef __TIMER_H__
#define __TIMER_H__

#include "fifo.h"

#define MAX_TIMER  (16)

typedef struct TIMER {
  struct TIMER* next_timer;
  unsigned int timeout, flags;
  FIFO* fifo;
  int data;
} TIMER;

typedef struct {
  unsigned int count, next_time;
  TIMER* t0;
  TIMER timers0[MAX_TIMER];
} TIMERCTL;

extern TIMERCTL timerctl;

void init_pit(void);
TIMER* timer_alloc(void);
void timer_free(TIMER* timer);
void timer_init(TIMER* timer, FIFO* fifo, int data);
void timer_settime(TIMER* timer, unsigned int timeout);

#endif
