#ifndef h_mem_
#define h_mem_

struct Memory {
	/* arena allocator backings */
	char *arena_tempAlloc_backing;
	char *arena_scratchAlloc_backing;
	char *arena_persistAlloc_backing;
	char *arena_IRAlloc_backing;

	/* stack backings */
	char *stack_callStack_stack;
	char *stack_callStack_frames;
};

#endif
