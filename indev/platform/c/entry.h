#ifndef PLATFORM_ENTRY_H_
#define PLATFORM_ENTRY_H_

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
Stack *initstack
(unsigned long capacity);

void freestack
(Stack *stack);

void pushframe
(Stack *stack, StackFrame *frame);

StackFrame popframe
(Stack *stack);

StackFrame readframe
(Stack *stack);

Args initargs
(int argc, char **argv);

#endif
