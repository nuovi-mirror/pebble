#ifndef ALLOCATOR_MEMORY_H_
#define ALLOCATOR_MEMORY_H_

#include "limits.h"

/* arena allocator backings */
extern char mem_arena_tempAlloc_backing[limits_allocator_temp_maxmem];
extern char mem_arena_scratchAlloc_backing[limits_allocator_scratch_maxmem];
extern char mem_arena_persistAlloc_backing[limits_allocator_persist_maxmem];
extern char mem_arena_IRAlloc_backing[limits_instructions_maxbuffersize];

/* stack backings */
extern char mem_stack_callStack[limits_stack_callStack_size];

#endif
