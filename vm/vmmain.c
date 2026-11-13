/* bit to run the VM itself */

#include "vm.h"
#include "allocator.h"
#include "ffi.h"
#include "findnewline.h"
#include "functions.h"
#include "instructionmapper.h"
#include "instructions.h"
#include "main.h"
#include "snprint.h"
#include "skipspace.h"
#include "variables.h"
#include "mem.h"
#include "limits.h"
#include "cli.h"
#include "clock.h"
#include "getprogram.h"

#ifdef debug
#include "lalloc.h" /* XXX debug */
#include "print.h" /* XXX debug */
#endif

/* Args and Stack should be defined by the platform entry code
 * which will include this file */
int vmmain 
(Args cliargs, Stack *stack, unsigned long long seed, struct Memory *mem, struct Limits *limits)
{
	/* init */
	struct Arena *tempAlloc = initAlloc(mem->arena_tempAlloc_backing, limits->allocator_temp_maxmem);
	struct Arena *persistAlloc = initAlloc(mem->arena_persistAlloc_backing, limits->allocator_persist_maxmem);
	struct Arena *scratchAlloc = initAlloc(mem->arena_scratchAlloc_backing, limits->allocator_scratch_maxmem);
	struct Arena *IRAlloc = initAlloc(mem->arena_IRAlloc_backing, limits->allocator_IRAlloc_maxmem);

	/* XXX read this pls
	 * tempAlloc may be freed once after every instruction
	 * scratchAlloc may be freed in-between function calls
	 * persistAlloc may never be freed until the VM exits
	 */

	VarMap vars = initVars(limits->variables_max, persistAlloc);
	FFIvars *ffivars = alloc(persistAlloc, sizeof(FFIvars));
	FuncMap funcs = initFuncs(limits->functions_max, persistAlloc);
	InstructionMap instructionMap = initInstructionMap(limits->instructions_maxcache, persistAlloc);

	Scheduler scheduler;
	scheduler.tasks = alloc(persistAlloc, (sizeof(Task) * limits->tasks_max));

	scheduler_init(&scheduler, stack, persistAlloc, limits);

	ffivars->map = &vars;
	ffivars->count = 0;
	ffivars->max = limits->variables_max;
	ffivars->namesize = limits->instructions_varnamesize;

	cli(&cliargs);
	Program fprogram = getprogram(limits->misc_maxfilebuffersize, &cliargs);
	char *filedata = fprogram.data;

	char *line;
	unsigned long current_instruction_count = 0;
	Instruction *program = alloc(IRAlloc, limits->instructions_maxbuffersize);

	/* startup */
	while ((line = findnewline(&filedata)) != NULL) 
	{
		if (current_instruction_count >= limits->instructions_max) error(errfatal, errcodelimitreached, "Instruction cap reached!");
		
		char *p = skipspace(line);
		if (p == NULL) continue;

		program[current_instruction_count] = makeIR(line, limits, tempAlloc, persistAlloc, &instructionMap);
		current_instruction_count++;
	}

	optimizeIR(program, &current_instruction_count, tempAlloc, persistAlloc);

	/* clean the file backing buffer */
	resetAllocator(tempAlloc);

	/* initalize VM CLI arguments */
	for (unsigned long i = 0; i < cliargs.count; i++)
	{
		char *buff = alloc(persistAlloc, limits->misc_vmargmaxdigits + 5);
		snprint(buff, sizeof(buff), "VMARG%u", i);

		Value *arg = alloc(persistAlloc, sizeof(Value));
		*arg = (Value){ .Type = type_str, .as.str = cliargs.values[i] };
		putVar(&vars, buff, arg, persistAlloc);
		
	}

	Value *argcount = alloc(persistAlloc, sizeof(Value));
	*argcount = (Value){ .Type = type_word, .as.word = cliargs.count };
	putVar(&vars, "VMARGC", argcount, persistAlloc);

	/* initialize the clock */
	Clock clock;
	timer_init(&clock);
	timer_start(&clock);

	/* execution */
	while (scheduler_next(&scheduler, limits))
	{
		Task *task = &scheduler.tasks[scheduler.current];
		scheduler.yield = 0;

		Value *self = getVar(&vars, "__task");

		if (self == NULL)
		{
			self = alloc(persistAlloc, sizeof(Value));
			putVar(&vars, "__task", self, persistAlloc);
		}

		*self = (Value){ .Type = type_word, .as.word = scheduler.current };

		while(!scheduler.yield && task->pc < current_instruction_count)
		{
			task->pc = interpret(&program[task->pc], program, seed, current_instruction_count, limits, &vars, ffivars, task->stack, &funcs, task->pc, persistAlloc, scratchAlloc, tempAlloc, &scheduler);

			resetAllocator(scratchAlloc);
			resetAllocator(tempAlloc);

			timer_update(&clock);
			if (limits->time_max != 0) /* dont check if time is unlimted */
				if (clock.time > limits->time_max) error(errfatal, errcodelimitreached, "Time is up!");
		}

		if (task->pc >= current_instruction_count && task->state != task_sleeping) scheduler_finish(&scheduler, &vars);
	}

	/* clean-up */
	freeAllocator(persistAlloc);
	freeAllocator(tempAlloc);
	freeAllocator(scratchAlloc);
	freeAllocator(IRAlloc);
	/* lfree(ffivars); XXX hack */
	freeVars(&vars);
	freeInstructionMap(&instructionMap);
	freeFuncs(&funcs);

#ifdef debug
	char buff[512];
	unsigned long allocated = lallocTracker / 1024;
	snprint(buff, 512, "%lu%s\n", allocated, "K allocated");
	print(buff);
#endif

	return 0;
}
