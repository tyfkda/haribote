#include "mtask.h"
#include "bootpack.h"
#include "dsctbl.h"
#include "stdio.h"  // NULL

TIMER* task_timer;
static TASKCTL* taskctl;

TASK* task_init(MEMMAN* memman) {
  SEGMENT_DESCRIPTOR* gdt = (SEGMENT_DESCRIPTOR*)ADR_GDT;
  taskctl = (TASKCTL*)memman_alloc_4k(memman, sizeof(TASKCTL));
  for (int i = 0; i < MAX_TASKS; ++i) {
    TASK* task = &taskctl->tasks0[i];
    task->flags = 0;
    task->sel = (TASK_GDT0 + i) * 8;
    set_segmdesc(gdt + TASK_GDT0 + i, 103, (int)&task->tss, AR_TSS32);
  }

  TASK* task = task_alloc();  // Main task.
  task->flags = 2;  // Running.
  taskctl->running = 1;
  taskctl->now = 0;
  taskctl->tasks[0] = task;
  load_tr(task->sel);
  task_timer = timer_alloc();
  timer_settime(task_timer, 2);
  return task;
}

TASK* task_alloc() {
  for (int i = 0; i < MAX_TASKS; ++i) {
    TASK* task = &taskctl->tasks0[i];
    if (task->flags != 0)
      continue;
    task->flags = 1;  // Used.
    task->tss.eip = 0;
    task->tss.eflags = 0x00000202; /* IF = 1; */
    task->tss.eax = task->tss.ecx = task->tss.edx = task->tss.ebx = 0;
    task->tss.ebp = task->tss.esi = task->tss.edi = 0;
    task->tss.es = task->tss.ds = task->tss.fs = task->tss.gs = 0;
    task->tss.ldtr = 0;
    task->tss.iomap = 0x40000000;
    //task->tss.cs = 2 * 8;
    //task->tss.esp = task_b_esp;
    //task->tss.ss = 0;
    return task;
  }
  return NULL;
}

void task_run(TASK* task) {
  task->flags = 2;  // Running.
  taskctl->tasks[taskctl->running++] = task;
}

void task_switch(void) {
  timer_settime(task_timer, 2);
  if (taskctl->running >= 2) {
    if (++taskctl->now == taskctl->running)
      taskctl->now = 0;
    farjmp(0, taskctl->tasks[taskctl->now]->sel);
  }
}

void task_sleep(TASK* task) {
  if (task->flags != 2)
    return;

  // TODO: Need to prevent interrupt?

  char ts = 0;  // Need task switch?
  if (task == taskctl->tasks[taskctl->now])
    ts = 1;
  // Find task.
  int i;
  for (i = 0; i < taskctl->running; ++i)
    if (taskctl->tasks[i] == task)
      break;
  --taskctl->running;
  if (i < taskctl->now)
    --taskctl->now;
  for (; i < taskctl->running; ++i)
    taskctl->tasks[i] = taskctl->tasks[i + 1];
  task->flags = 1;  // Not running.
  if (!ts)
    return;

  if (taskctl->now >= taskctl->running)
    taskctl->now = 0;
  farjmp(0, taskctl->tasks[taskctl->now]->sel);
}

void task_wake(TASK* task) {
  if (task->flags == 1)
    task_run(task);
}
