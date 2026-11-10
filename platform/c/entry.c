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

	cliargs.count = (unsigned long)argc;
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

/* bits and bobs and such */
struct SizingCosts {
	unsigned long bytes_per_line;		/* average line of source */
	unsigned long temp_per_instruction;	/* temp left behind by parsing one line */
	unsigned long temp_slack;
	unsigned long persist_fixed;		/* hash tables, FFI vars, args, ... */
	unsigned long persist_per_instruction;	/* operand strings kept by the IR */
	unsigned long persist_per_cached;	/* an instruction cache entry */
	unsigned long persist_per_task;		/* task bookkeeping, besides its stack */
	unsigned long persist_data;		/* variables, functions, strings made at runtime */
	unsigned long scratch_strings;
};

static const struct SizingCosts default_costs = {
	.bytes_per_line = 32,
	.temp_per_instruction = 24,
	.temp_slack = 64 * 1024,
	.persist_fixed = 128 * 1024,
	.persist_per_instruction = 40,
	.persist_per_cached = 160,
	.persist_per_task = 64,
	.persist_data = 2 * 1024 * 1024,
	.scratch_strings = 512,
};

struct PackedLimits setlimits
(void)
{
	struct Limits limits = {0};
	struct Memory mem = {0};
	struct PackedLimits packed = {0};
	const struct SizingCosts *costs = &default_costs; /* an embedder could take this as a parameter */

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

	limits.misc_maxfilebuffersize = (limits.instructions_max * costs->bytes_per_line);
	limits.misc_vmargmaxdigits = 2;
	limits.misc_strlen = limits.cmd_args * limits.cmd_chars;
	limits.misc_dirlist_maxsize = limits.misc_strlen * 12;

	limits.file_escapelen_add = 512;
	limits.file_maxsize = (12 * 1024 * 1024);

	limits.instruction_new_destsize = 32;
	limits.instruction_new_datasize = 32;

	limits.expr_maxdepth = 32;
	limits.expr_currdepth = 0;

	limits.allocator_temp_maxmem = (limits.misc_maxfilebuffersize + (limits.instructions_max * costs->temp_per_instruction) + costs->temp_slack);

	limits.allocator_persist_maxmem = (costs->persist_fixed 
		+ (limits.instructions_max * costs->persist_per_instruction)
		+ (limits.instructions_maxcache * costs->persist_per_cached)
		+ (limits.tasks_max * ((limits.tasks_depth * sizeof(StackFrame)) + costs->persist_per_task))
		+ costs->persist_data);

	limits.allocator_scratch_maxmem = ((costs->scratch_strings * limits.misc_strlen));
	limits.allocator_IRAlloc_maxmem = ((limits.instructions_maxbuffersize));
	limits.allocator_temp_maxmem = arenaBackingSize(limits.allocator_temp_maxmem);
	limits.allocator_persist_maxmem = arenaBackingSize(limits.allocator_persist_maxmem);
	limits.allocator_scratch_maxmem = arenaBackingSize(limits.allocator_scratch_maxmem);
	limits.allocator_IRAlloc_maxmem = arenaBackingSize(limits.allocator_IRAlloc_maxmem);
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
