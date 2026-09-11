/* portable entry point helper (C) */

#include "entry.h"
#include "print.h"
#include "exitproc.h"
#include "lalloc.h"
#include "lfree.h"

/* temporary NULL definition we can use */
#undef NULL
#define NULL ((void *)0)

/* stack helpers */
void pushframe
(Stack *stack, StackFrame *frame) 
{
	if (stack->count >= stack->capacity) 
	{
		print("ERROR: CALLSTACK: STACK OVERFLOW!\n");
		exitproc(1);
	}

	stack->items[stack->count] = *frame;
	stack->count++;
}

StackFrame popframe
(Stack *stack) 
{
	StackFrame frame;

	if (stack->count == 0) 
	{
		print("ERROR: CALLSTACK: STACK UNDERFLOW!\n");
		exitproc(1);
	}

	stack->count--;
	frame = stack->items[stack->count];
	
	return frame;
}

StackFrame readframe
(Stack *stack) 
{
	StackFrame frame;

	if (stack->count == 0) 
	{
		print("ERROR: CALLSTACK: STACK UNDERFLOW!\n");
		exitproc(1);
	}

	return frame;
}

Args initargs
(int argc, char **argv) 
{
	Args cliargs; /* init argument struct */

	cliargs.count = (unsigned long)argc; /* set the count */
	cliargs.values = argv; /* set the arguments */

	return cliargs;
}

Stack *initstack
(unsigned long capacity) 
{
	/* initalize the stack */
	StackFrame *stack_items = lalloc(capacity * sizeof(*stack_items));
		/* max number of bytes for the stack */

	Stack *stack = lalloc(sizeof(Stack));

	if (stack == NULL || stack_items == NULL || capacity == 0)
	{
		print("ERROR: INIT: CANNOT ALLOCATE A CALL STACK!\n");
		exitproc(1);
	}

	stack->items = stack_items;
	stack->count = 0;
	stack->capacity = capacity; /* max number of entries on the stack at a time */

	return stack;
}

void freestack
(Stack *stack)
{
	lfree(stack->items);
	lfree(stack);
}	

/* remove it */
#undef NULL
