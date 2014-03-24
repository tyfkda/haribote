#ifndef __MTASK_H__
#define __MTASK_H__

#include "dsctbl.h"
#include "fifo.h"

struct CONSOLE;
struct MEMMAN;

#define MAX_TASKS       (20)
#define MAX_TASKS_LV    (8)
#define MAX_TASKLEVELS  (5)
#define TASK_GDT0  (3)

typedef struct {
  int backlink, esp0, ss0, esp1, ss1, esp2, ss2, cr3;
  int eip, eflags, eax, ecx, edx, ebx, esp, ebp, esi, edi;
  int es, cs, ss, ds, fs, gs;
  int ldtr, iomap;
} TSS32;

typedef struct TASK {
  int sel, flags;  // sel = GDT number.
  int level, priority;
  FIFO fifo;
  TSS32 tss;
  int fpu[108 / 4];
  SEGMENT_DESCRIPTOR ldt[2];
  struct CONSOLE* cons;
  int ds_base;
  void* cons_stack;
  struct FDHANDLE* fhandle;
  int fhandleCount;
  char** argv;
} TASK;

typedef struct {
  int running;
  int now;
  TASK* tasks[MAX_TASKS_LV];
} TASKLEVEL;

typedef struct TASKCTL {
  int now_lv;
  TASK* task_fpu;
  char lv_change;
  TASKLEVEL level[MAX_TASKLEVELS];
  TASK tasks0[MAX_TASKS];
} TASKCTL;

extern struct TIMER* task_timer;
extern struct TASKCTL* taskctl;

TASK* task_init(struct MEMMAN* memman);
TASK* task_alloc();
void task_free(TASK* task);
TASK* task_now(void);
void task_run(TASK* task, int level, int priority);
void task_switch(void);
void task_sleep(TASK* task);
void task_wake(TASK* task);
struct FDHANDLE* task_get_free_fhandle(TASK* task);

#endif
