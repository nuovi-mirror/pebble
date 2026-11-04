#ifndef h_mem_
#define h_mem_

#include "main.h"

struct Memory {
	/* arena allocator backings */
	char *arena_tempAlloc_backing;
	char *arena_scratchAlloc_backing;
	char *arena_persistAlloc_backing;
	char *arena_IRAlloc_backing;

	/* stack backings */
	Stack *stack_callStack_stack;
	StackFrame *stack_callStack_frames;
};

#endif
