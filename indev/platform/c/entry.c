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
/* measured costs, in bytes. see the notes on how to re-measure them with _GETMEM */
#define LIMITS_BYTES_PER_LINE 32		/* average line of source */
#define LIMITS_TEMP_PER_INSTRUCTION 24		/* temp left behind by parsing one line */
#define LIMITS_TEMP_SLACK (64 * 1024)
#define LIMITS_PERSIST_FIXED (128 * 1024)	/* hash tables, FFI vars, args, ... */
#define LIMITS_PERSIST_PER_INSTRUCTION 40	/* operand strings kept by the IR */
#define LIMITS_PERSIST_PER_CACHED 160		/* an instruction cache entry */
#define LIMITS_PERSIST_PER_TASK 64		/* task bookkeeping, besides its stack */
#define LIMITS_PERSIST_DATA (2 * 1024 * 1024)	/* variables, functions, strings made at runtime */
#define LIMITS_SCRATCH_STRINGS 512

struct PackedLimits setlimits
(void)
{
	struct Limits limits = {0};
	struct Memory mem = {0};
	struct PackedLimits packed = {0};

	/* the knobs: what a program is allowed to be. the arena sizes below are worked 
	 * out from these, so they cannot disagree with each other. a program that is 
	 * too big then gets a clear 'cap reached' error, instead of 'Out of memory!' 
	 * at some arbitrary point. for a small target, lower instructions_max, 
	 * tasks_max and tasks_depth; everything else follows */
	limits.instructions_max = 200000;
	limits.tasks_max = 32;
	limits.tasks_depth = 256;

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

	limits.cmd_args = 8;
	limits.cmd_chars = 64;

	limits.escapes_namesize = 32;

	limits.misc_maxfilebuffersize = (limits.instructions_max * LIMITS_BYTES_PER_LINE);
	limits.misc_vmargmaxdigits = 2;
	limits.misc_strlen = limits.cmd_args * limits.cmd_chars;

	limits.instruction_new_destsize = 32;
	limits.instruction_new_datasize = 32;

	/* temp holds the whole source file while it is parsed, plus what parsing leaves behind */
	limits.allocator_temp_maxmem = (limits.misc_maxfilebuffersize 
		+ (limits.instructions_max * LIMITS_TEMP_PER_INSTRUCTION) + LIMITS_TEMP_SLACK + sizeof(struct Arena)); 

	/* persist is never freed. it holds everything a program keeps: what loading it costs, 
	 * the instruction cache, a stack for every task, and room for its variables, functions 
	 * and strings. the VM cannot give any of it back, so the sum has to be right */
	limits.allocator_persist_maxmem = (LIMITS_PERSIST_FIXED 
		+ (limits.instructions_max * LIMITS_PERSIST_PER_INSTRUCTION)
		+ (limits.instructions_maxcache * LIMITS_PERSIST_PER_CACHED)
		+ (limits.tasks_max * ((limits.tasks_depth * sizeof(StackFrame)) + LIMITS_PERSIST_PER_TASK))
		+ LIMITS_PERSIST_DATA + sizeof(struct Arena));

	/* scratch is reset after every instruction; this many full-size strings is plenty */
	limits.allocator_scratch_maxmem = ((LIMITS_SCRATCH_STRINGS * limits.misc_strlen) + sizeof(struct Arena));
	limits.allocator_IRAlloc_maxmem = ((limits.instructions_maxbuffersize) + sizeof(struct Arena));
	limits.allocator_lalloc_maxretries = 3;


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
