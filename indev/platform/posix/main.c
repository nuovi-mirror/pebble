#include "main.h"
#include "mem.h"
#include "limits.h"
#include "vm.h"
#include <stdlib.h>

int main 
(int argc, char **argv)
{
	struct Limits limits;
	struct Memory mem;

	limits.instructions_max = 500000;
	limits.instructions_maxbuffersize = (limits.instructions_max * sizeof(Instruction));
	limits.instructions_initbuffersize = (512 * sizeof(Instruction));
	limits.instructions_maxcache = 512;
	limits.instructions_varnamesize = 64;
	limits.stack_callStack_cap = 512;
	limits.stack_callStack_frameSize = (sizeof(StackFrame));
	limits.stack_callStack_size = (limits.stack_callStack_cap * limits.stack_callStack_frameSize);
	limits.functions_max = 5000;
	limits.functions_namesize = 32;
	limits.variables_max = 512;
	limits.escapes_namesize = 32;
	limits.misc_maxfilebuffersize = (6 * 1024 * 1024);
	limits.instruction_new_destsize = 32;
	limits.instruction_new_datasize = 32;

	limits.allocator_temp_maxmem = ((8 * 1024 * 1024) + sizeof(struct Arena)); 
	limits.allocator_persist_maxmem = ((32 * 1024 * 1024) + sizeof(struct Arena));
	limits.allocator_scratch_maxmem = ((6 * 1024 * 1024) + sizeof(struct Arena));
	limits.allocator_IRAlloc_maxmem = ((limits.instructions_maxbuffersize) + sizeof(struct Arena));

	char *mem_arena_tempAlloc_backing = malloc(limits.allocator_temp_maxmem);
	char *mem_arena_scratchAlloc_backing = malloc(limits.allocator_scratch_maxmem);
	char *mem_arena_persistAlloc_backing = malloc(limits.allocator_persist_maxmem);
	char *mem_arena_IRAlloc_backing = malloc(limits.allocator_IRAlloc_maxmem);
	Stack *mem_stack_callStack_stack = malloc(limits.stack_callStack_size);
	StackFrame *mem_stack_callStack_frames = malloc(limits.stack_callStack_frameSize  
		* limits.stack_callStack_cap);

	mem.arena_tempAlloc_backing = mem_arena_tempAlloc_backing;
	mem.arena_scratchAlloc_backing = mem_arena_scratchAlloc_backing;
	mem.arena_persistAlloc_backing = mem_arena_persistAlloc_backing;
	mem.arena_IRAlloc_backing = mem_arena_IRAlloc_backing;
	mem.stack_callStack_stack = mem_stack_callStack_stack;
	mem.stack_callStack_frames = mem_stack_callStack_frames;

	unsigned long long seed = (unsigned long long)rand();
	Args cliargs = initargs(argc, argv);
	Stack *stack = initstack(1024, &mem);

	int ret = vmmain(cliargs, stack, seed, &mem, &limits);
	freestack(stack);
	return ret;
}
