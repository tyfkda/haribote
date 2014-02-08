#ifndef __MTASK_H__
#define __MTASK_H__

#include "timer.h"

extern TIMER* mt_timer;

void mt_init(void);
void mt_taskswitch(void);

#endif
