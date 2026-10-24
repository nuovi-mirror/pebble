/* portable entry point helper (C) */

#include "entry.h"
#include "exitproc.h"
#include "main.h"
#include "mem.h"
#include "snprint.h"
#include "print.h"
#include "allocator.h"
#include "instructions.h"
#include "limits.h"
#include "lalloc.h"
#include "indrng.h" /* imported random number generator code */

/* temporary NULL definition we can use */
#undef NULL
#define NULL ((void *)0)

/* error helper */
void error
(unsigned int severity, unsigned int code, const char *msg)
{
	switch (severity)
	{
		case errfatal: print("A fatal error has occurred!\n"); break;
		case errmajor: print("A major error has occurred!\n"); break;
		case errsevere: print("A severe error has occurred!\n"); break;
		case errwarning: print("A nonfatal error has occurred!\n"); break;
		case errnonfatal: print("A nonfatal errror has occureed!\n"); break;
	}

	char codebuff[32];
	char severitybuff[32];

	snprint(&codebuff[0], sizeof(codebuff), "%i", code);
	snprint(&severitybuff[0], sizeof(severitybuff), "%i", severity);

	print("Code: ");
	print(&codebuff[0]);
	print("\nSeverity: ");
	print(&severitybuff[0]);
	print("\nMessage: ");
	print(msg);
	print("\n");

	if (severity == errfatal) exitproc((int)code);
}


/* stack helpers */
void pushframe 
(Stack *stack, StackFrame *frame)
{
	if (stack->count >= stack->capacity) error(errfatal, errcodestackerror, "Call Stack overflow!");

	stack->items[stack->count] = *frame;
	stack->count++;
}

StackFrame popframe 
(Stack *stack)
{
	StackFrame frame;

	if (stack->count <= 0) error(errfatal, errcodestackerror, "Call Stack underflow!");

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

	if (stack == NULL || stack_items == NULL || capacity == 0) error(errfatal, errcodestackerror, "Cannot allocate a call stack!");

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

/* helper for default limits values */
struct PackedLimits setlimits
(void)
{
	struct Limits limits = {0};
	struct Memory mem = {0};
	struct PackedLimits packed = {0};

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
	limits.misc_vmargmaxdigits = 2;
	limits.instruction_new_destsize = 32;
	limits.instruction_new_datasize = 32;

	limits.allocator_temp_maxmem = ((32 * 1024 * 1024) + sizeof(struct Arena)); 
	limits.allocator_persist_maxmem = ((256 * 1024 * 1024) + sizeof(struct Arena));
	limits.allocator_scratch_maxmem = ((32 * 1024 * 1024) + sizeof(struct Arena));
	limits.allocator_IRAlloc_maxmem = ((limits.instructions_maxbuffersize) + sizeof(struct Arena));
	limits.allocator_lalloc_maxretries = 3;
	limits.tasks_max = 32;
	limits.tasks_depth = 256;

	char *mem_arena_tempAlloc_backing = lalloc(limits.allocator_temp_maxmem, limits.allocator_lalloc_maxretries);
	char *mem_arena_scratchAlloc_backing = lalloc(limits.allocator_scratch_maxmem, limits.allocator_lalloc_maxretries);
	char *mem_arena_persistAlloc_backing = lalloc(limits.allocator_persist_maxmem, limits.allocator_lalloc_maxretries);
	char *mem_arena_IRAlloc_backing = lalloc(limits.allocator_IRAlloc_maxmem, limits.allocator_lalloc_maxretries);
	Stack *mem_stack_callStack_stack = lalloc(limits.stack_callStack_size, limits.allocator_lalloc_maxretries);
	StackFrame *mem_stack_callStack_frames = lalloc(limits.stack_callStack_frameSize * limits.stack_callStack_cap, limits.allocator_lalloc_maxretries);

	mem.arena_tempAlloc_backing = mem_arena_tempAlloc_backing;
	mem.arena_scratchAlloc_backing = mem_arena_scratchAlloc_backing;
	mem.arena_persistAlloc_backing = mem_arena_persistAlloc_backing;
	mem.arena_IRAlloc_backing = mem_arena_IRAlloc_backing;
	mem.stack_callStack_stack = mem_stack_callStack_stack;
	mem.stack_callStack_frames = mem_stack_callStack_frames;

	packed.limits = limits;
	packed.memory = mem;

	return packed;
}
