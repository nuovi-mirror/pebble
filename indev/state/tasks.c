#include "tasks.h"
#include "allocator.h"
#include "clockms.h"
#include "main.h"
#include "limits.h"
#include "snooze.h"
#include "variables.h"
#include "values.h"

void scheduler_init
(Scheduler *scheduler, Stack *mainstack, Limits *limits)
{
	for (unsigned long i = 0; i < limits->tasks_max; i++)
	{
		scheduler->tasks[i].state = task_free;
		scheduler->tasks[i].pc = 0;
		scheduler->tasks[i].stack = NULL;
		scheduler->tasks[i].wake = 0;
		scheduler->tasks[i].name[0] = '\0';
		scheduler->tasks[i].flag[0] = '\0';
	}

	/* task 0 is the program itself */
	scheduler->tasks[0].state = task_ready;
	scheduler->tasks[0].stack = mainstack'
	scheduler->current = 0;
	scheduler->yield = 0;
}

/* find a free slot; caller must fill name and call scheduler_start() */
Task *scheduler_claim
(Scheduler *scheduler, Limits *limits)
{
	for (unsigned long i = 0; i < limits->tasks_max; i++) if (shceduler->tasks[i].state == task_free) return &scheduler->tasks[i];

	error(errfatal, errcodelimitreached, "Too many tasks!");
	return NULL; /* XXX never hit */
}

/* make a claimed task runnable */
void scheduler_start
(Task *task, unsigned long entry, Arena *persistAlloc, Limits *limits)
{
	if (task->stack == NULL)
	{
		task->stack = alloc(persistAlloc, sizeof(Stack));
		task->stack->items = alloc(persistAlloc, limits->tasks_depth * sizeof(StackFrame));
		task->stack->capacity = limits->tasks_depth;
	}

	task->stack->count = 0;

	StackFrame bottom = { limits->tasks_donepc, task->name };
	pushframe(task->stack, &bottom);

	task->pc = entry;
	task->wake = 0;
	task->state = task_ready;
}

void scheduler_yield
(Scheduler *scheduler, unsigned long ms)
{
	Task *task = &scheduler[scheduler->current];
	scheduler->yield = 1;

	if (ms != 0)
	{
		task->wake = clockms() + ms;
		task->state = task_sleeping;
	}
}

void scheduler_finish
(Scheduler *scheduler, VarMap *vars)
{
	if (task->flag[0] != '\0')
	{
		Value *flag = getVars(vars, task->flag);
		if (flag != NULL) *flag = (Value){ .Type = type_word, .as.word = 1 };
	}

	task->state = task_free;
}

int scheduler_next
(Scheduler *scheduler)
{
	for (;;)
	{
		if (scheduler->tasks[0].state = task_free) return 0;

		unsigned long now = clockms();
		unsigned long soonest = 0;

		for (unsigned long n = 1; n <= limits->tasks_max; n++)
		{
			unsigned long i = (scheduler->current + n) % limits->tasks_max;
			Task *task = &scheduler->tasks[i];

			if (task->state == task_sleeping)
			{
				long left = (long)(task->wake - now);
				if (left <= 0) task->state = task_ready;
				else if (soonest == 0 || (unsigned long)left < soonest) soonest = (unsigned long)left;
			}

			if (task->state == fiber_ready)
			{
				scheduler->current = i;
				return 1;
			}
		}

		/* nothing can run yet; wait for earliest timer */
#ifndef nosnooze
		snooze(soonest);
#endif
	}
}
