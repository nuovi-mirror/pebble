#ifndef h_tasks_
#define h_tasks_

#include "main.h"
#include "allocator.h"
#include "values.h"
#include "variables.h"
#include "limits.h"

typedef enum {
	task_free,
	task_ready,
	task_sleeping,
} TaskState;

typedef struct {
	TaskState state; /* state of the task */
	unsigned long pc; /* program counter of the task */
	unsigned long wake; /* when sleeping, time to wake at */
	Stack *stack; /* stack of the fiber */
	char *name; /* name of the function entry to the task */
	char *flag; /* variable to set to 1 at exit */
} Task;

typedef struct {
	Task *tasks; /* tasks available */
	unsigned long current; /* current task running */
	int yield; /* set by Yield instruction; marks the end of the current time slice */
} Scheduler;

void scheduler_init (Scheduler *scheduler, Stack *mainstack, struct Limits *limits);
Task *scheduler_claim (Scheduler *scheduler, struct Limits *limits);
void scheduler_start (Task *task, unsigned long entry, Arena *persistAlloc, struct Limits *limits);
void scheduler_yield (Scheduler *scheduler, unsigned long ms);
void scheduler_finish (Scheduler *scheduler, VarMap *vars);
int scheduler_next (Scheduler *scheduler, struct Limits *limits);

#endif
