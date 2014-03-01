#include "mtask.h"
#include "bootpack.h"
#include "dsctbl.h"
#include "file.h"
#include "memory.h"
#include "stdio.h"  // NULL
#include "timer.h"

enum TaskFlag {
  FREE = 0,
  ALLOCATED = 1,
  RUNNING = 2,
};

TIMER* task_timer;
TASKCTL* taskctl;

static void task_idle(void) {
  for (;;)
    io_hlt();
}

TASK* task_now(void) {
  TASKLEVEL *tl = &taskctl->level[taskctl->now_lv];
  return tl->tasks[tl->now];
}

static void task_add(TASK *task) {
  TASKLEVEL *tl = &taskctl->level[task->level];
  tl->tasks[tl->running++] = task;
  task->flags = RUNNING;
}

static void task_remove(struct TASK *task) {
  TASKLEVEL *tl = &taskctl->level[task->level];

  // Finds task.
  int i;
  for (i = 0; i < tl->running; i++)
    if (tl->tasks[i] == task)
      break;

  --tl->running;
  if (i < tl->now)
    --tl->now;
  if (tl->now >= tl->running)
    tl->now = 0;
  task->flags = ALLOCATED;  // Sleep.

  // Shift
  for (; i < tl->running; ++i)
    tl->tasks[i] = tl->tasks[i + 1];
}

static void task_switchsub(void) {
  int i;
  for (i = 0; i < MAX_TASKLEVELS; ++i) {
    if (taskctl->level[i].running > 0)
      break;
  }
  taskctl->now_lv = i;
  taskctl->lv_change = 0;
}

TASK* task_init(MEMMAN* memman) {
  SEGMENT_DESCRIPTOR* gdt = (SEGMENT_DESCRIPTOR*)ADR_GDT;
  taskctl = (TASKCTL*)memman_alloc_4k(memman, sizeof(TASKCTL));
  for (int i = 0; i < MAX_TASKS; ++i) {
    taskctl->tasks0[i].flags = 0;
    taskctl->tasks0[i].sel = (TASK_GDT0 + i) * 8;
    taskctl->tasks0[i].tss.ldtr = (TASK_GDT0 + MAX_TASKS + i) * 8;
    set_segmdesc(gdt + TASK_GDT0 + i, 103, (int)&taskctl->tasks0[i].tss, AR_TSS32);
    set_segmdesc(gdt + TASK_GDT0 + MAX_TASKS + i, 15, (int)taskctl->tasks0[i].ldt, AR_LDT);
  }
  for (int i = 0; i < MAX_TASKLEVELS; ++i) {
    taskctl->level[i].running = 0;
    taskctl->level[i].now = 0;
  }

  TASK* task = task_alloc();  // Main task.
  task->flags = RUNNING;
  task->priority = 2;  // 0.02 sec
  task->level = 0;  // Max level.
  task_add(task);
  task_switchsub();
  load_tr(task->sel);
  task_timer = timer_alloc();
  timer_settime(task_timer, task->priority);

  TASK* idle = task_alloc();
  idle->tss.esp = (int)memman_alloc_4k(memman, 256) + 256;
  idle->tss.eip = (int)&task_idle;
  idle->tss.es = idle->tss.ss = idle->tss.ds = idle->tss.fs = idle->tss.gs = 1 * 8;
  idle->tss.cs = 2 * 8;
  task_run(idle, MAX_TASKLEVELS - 1, 1);

  taskctl->task_fpu = NULL;

  return task;
}

TASK* task_alloc() {
  for (int i = 0; i < MAX_TASKS; ++i) {
    TASK* task = &taskctl->tasks0[i];
    if (task->flags != 0)
      continue;
    task->flags = ALLOCATED;  // Used.
    task->tss.eip = 0;
    task->tss.eflags = 0x00000202; /* IF = 1; */
    task->tss.eax = task->tss.ecx = task->tss.edx = task->tss.ebx = 0;
    task->tss.ebp = task->tss.esi = task->tss.edi = 0;
    task->tss.es = task->tss.ds = task->tss.fs = task->tss.gs = 0;
    task->tss.iomap = 0x40000000;
    task->tss.ss0 = 0;
    task->cons_stack = NULL;
    //task->tss.cs = 2 * 8;
    //task->tss.esp = task_b_esp;
    //task->tss.ss = 0;
    task->fpu[0] = 0x037f;  // CW (control word)
    task->fpu[1] = 0x0000;  // SW (status word)
    task->fpu[2] = 0xffff;  // TW (tag word)
    for (int i = 3; i < 108 / 4; ++i)
      task->fpu[i] = 0;
    return task;
  }
  return NULL;
}

void task_free(TASK* task) {
  task->flags = 0;
}

void task_run(TASK* task, int level, int priority) {
  if (level < 0)
    level = task->level;
  if (priority > 0)
    task->priority = priority;
  if (task->flags == RUNNING && task->level != level)  // Change running task level.
    task_remove(task);  // This makes the task ALLOCATED (sleep).
  if (task->flags != RUNNING) {
    task->level = level;
    task_add(task);
  }

  taskctl->lv_change = 1;
}

void task_sleep(TASK* task) {
  if (task->flags != RUNNING)
    return;

  // TODO: Need to prevent interrupt?

  TASK* now_task = task_now();
  task_remove(task);  // This makes the task ALLOCATED (sleep).
  if (task == now_task) {
    // Sleep by self => need task switch.
    task_switchsub();
    now_task = task_now();
    farjmp(0, now_task->sel);
  }
}

void task_switch(void) {
  if (taskctl->now_lv >= MAX_TASKLEVELS) {
    io_stihlt();
    return;
  }

  TASKLEVEL* tl = &taskctl->level[taskctl->now_lv];
  TASK* now_task = tl->tasks[tl->now++];
  if (tl->now >= tl->running)
    tl->now = 0;
  if (taskctl->lv_change) {
    task_switchsub();
    tl = &taskctl->level[taskctl->now_lv];
  }
  TASK* new_task = tl->tasks[tl->now];
  timer_settime(task_timer, new_task->priority);
  if (new_task != now_task)
    farjmp(0, new_task->sel);
}

void task_wake(TASK* task) {
  if (task->flags != RUNNING)
    task_run(task, -1, 0);
}

int* inthandler07(int *esp) {
  (void)esp;
  TASK *now = task_now();
  io_cli();
  io_clts();
  if (taskctl->task_fpu != now) {
    if (taskctl->task_fpu != NULL)
      io_fnsave(taskctl->task_fpu->fpu);
    io_frstor(now->fpu);
    taskctl->task_fpu = now;
  }
  io_sti();
  return 0;
}

FILEHANDLE* task_get_free_fhandle(TASK* task) {
  for (int i = 0; i < task->fhandleCount; ++i) {
    if (task->fhandle[i].finfo == NULL)
      return &task->fhandle[i];
  }
  return NULL;
}
