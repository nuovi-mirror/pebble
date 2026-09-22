#include "memory.h"
#include "limits.h"

char mem_arena_tempAlloc_backing[limits_allocator_temp_maxmem];
char mem_arena_scratchAlloc_backing[limits_allocator_scratch_maxmem];
char mem_arena_persistAlloc_backing[limits_allocator_persist_maxmem];
char mem_arena_IRAlloc_backing[limits_instructions_maxbuffersize];

char mem_stack_callStack[limits_stack_callStack_size];
