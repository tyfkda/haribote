#ifndef __TIMER_H__
#define __TIMER_H__

typedef struct {
  unsigned int count;
} TIMERCTL;

extern TIMERCTL timerctl;

void init_pit(void);

#endif
