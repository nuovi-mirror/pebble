/* portable entry point helper (C) */

#ifndef PLATFORM_ENTRY_H_
#define PLATFORM_ENTRY_H_

#include "print.h"
#include "exitproc.h"
#include "lalloc.h"
#include "lfree.h"

/* structs for the VM */
typedef struct Args 
{
	unsigned long count;
	char **values;
} Args;

typedef struct StackFrame 
{
	unsigned long return_pc;
	char *funcname; /* function name this frame belongs to */
} StackFrame;

typedef struct Stack 
{
	StackFrame *items;
	unsigned long count;
	unsigned long capacity;
} Stack;

/* stack helpers */
void pushframe
(Stack *stack, StackFrame frame) 
{
	if (stack->count >= stack->capacity) 
	{
		print("ERROR: CALLSTACK: STACK OVERFLOW!");
		exitproc(1);
	}

	stack->items[stack->count] = frame;
	stack->count++;
}

StackFrame popframe
(Stack *stack) 
{
	StackFrame frame;

	if (stack->count == 0) 
	{
		print("ERROR: CALLSTACK: STACK UNDERFLOW!");
		exitproc(1);
	}

	stack->count--;
	frame = stack->items[stack->count];
	
	return frame;
}

/* VM arguments follow a POSIX-esque structure */
/*   - Args struct */
/*     - size_t count = number of args */
/*     - char **values = array of arguments */
/* the last argument should be a null character */

Args initargs
(int argc, char **argv) 
{
	Args cliargs; /* init argument struct */

	cliargs.count = (unsigned long)argc; /* set the count */
	cliargs.values = argv; /* set the arguments */

	return cliargs;
}

Stack *initstack
(unsigned long stacksize, unsigned long stackmentries) 
{
	/* initalize the stack */
	StackFrame *stack_items = lalloc(stacksize * sizeof(*stack_items));
		/* max number of bytes for the stack */

	Stack *stack = lalloc(sizeof(Stack));

	if (stack == NULL)
	{
		print("ERROR: INIT: CANNOT ALLOCATE A CALL STACK!\n");
		exitproc(1);
	}

	stack->items = stack_items;
	stack->count = 0;
	stack->capacity = stackmentries; /* max number of entries on the stack at a time */

	return stack;
}

void *freestack
(Stack *stack)
{
	lfree(stack->items);
	lfree(stack);
}	

/* now you just need to provide a main() function in your platform entry */
/* that calls initstack and initargs and calls vmmain() */

/* example for POSIX + C

#undef NULL
#define NULL ((void *)0)

#include "../platform/use/entry.h"
#include "vm.c"

int main(int argc, char **argv) {
	Args cliargs = initargs(argc, argv);
	Stack *stack = initstack(1024, 1024);
	return vmmain(cliargs, stack);
	freestack(stack);
}

*/

#endif
