/* POSIX-specific program entry point */

#include <stddef.h>
#include <stdlib.h>

#include "../platform/use/print.h"

/* this is responsible for doing three things */
/*   - getting command-line arguments for the platform */
/*   - formatting the command-line arguments for the VM */
/*   - set up a call stack */
/*   - set up stack helpers */
/*   - calling the main VM entry at vmmain */

/* structs for the VM */
typedef struct Args {
	size_t count;
	char **values;
} Args;

typedef struct StackFrame {
	size_t return_pc;
} StackFrame;

typedef struct Stack {
	StackFrame *items;
	size_t count;
	size_t capacity;
} Stack;

/* stack helpers */
void pushframe(Stack *stack, StackFrame frame) {
	if (stack->count >= stack->capacity) {
		print("ERROR: CALLSTACK: STACK OVERFLOW!");
		exit(1);
	}

	stack->items[stack->count] = frame;
	stack->count++;
}

StackFrame popframe(Stack *stack) {
	StackFrame frame;

	if (stack->count == 0) {
		print("ERROR: CALLSTACK: STACK UNDERFLOW!");
		exit(1);
	}

	stack->count--;
	frame = stack->items[stack->count];
	
	return frame;
}

#include "vm.c" /* include after so the stuff is visible */

/* VM arguments follow a POSIX-esque structure */
/*   - Args struct */
/*     - size_t count = number of args */
/*     - char **values = array of arguments */
/* the last argument should be a null character */

int main(int argc, char **argv) {
	Args cliargs; /* init argument struct */

	cliargs.count = (size_t)argc; /* set the count */
	cliargs.values = argv; /* set the arguments */

	/* initalize the stack */
	StackFrame *stack_items = malloc(1024 * sizeof(*stack_items));
		/* max number of bytes for the stack */

	Stack stack = {
		stack_items,
		0,
		1024 /* max number of entries on the stack at a time */
	};

	return vmmain(cliargs, &stack); /* call main VM entry point */

	/* and our job here is done */
}
