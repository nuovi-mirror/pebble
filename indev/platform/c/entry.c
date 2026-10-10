/* portable entry point helper (C) */

#include "entry.h"
#include "exitproc.h"
#include "main.h"
#include "limits.h"
#include "mem.h"
#include "print.h"
#include "indrng.h" /* imported random number generator code */

/* temporary NULL definition we can use */
#undef NULL
#define NULL ((void *)0)


/* stack helpers */
void pushframe 
(Stack *stack, StackFrame *frame)
{
	if (stack->count >= stack->capacity) {
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

	if (stack->count == 0) {
		print("ERROR: CALLSTACK: STACK UNDERFLOW!\n");
		exitproc(1);
	}

	stack->count--;
	frame = stack->items[stack->count];

	return frame;
}

Args initargs 
(int argc, char **argv)
{
	Args cliargs;

	cliargs.count = (unsigned long)argc - 1;
	cliargs.values = argv;

	return cliargs;
}

Stack *initstack 
(unsigned long capacity, struct Memory *mem)
{
	StackFrame *stack_items = (StackFrame *)mem->stack_callStack_frames;
	Stack *stack = (Stack *)mem->stack_callStack_stack;

	if (stack == NULL || stack_items == NULL || capacity == 0) {
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
{ /* XXX this does nothing now */
}

#ifdef indrng
/* shim for the psedorandom helper */
unsigned long long getrandom
(void)
{
	struct seed seed;
	volatile unsigned long long sink = 0;
	unsigned long long temp[scount];
	unsigned long count = ecount;

	grandom(&seed, &sink);

	for (unsigned long i = 0; i < ecount; i++) temp[i] = seed.value[i];
	while (count > 1)
	{
		unsigned long next = 0;

		for (unsigned long i = 0; i + 1 < count; i += 2) temp[next++] = mix64(temp[i] ^ temp[i + 1]);
		if (count & 1) temp[next++] = temp[count - 1];

		count = next;
	}

	return mix64(temp[0]);
}
#endif
