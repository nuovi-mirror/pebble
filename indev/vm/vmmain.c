/* bit to run the VM itself */

#include "vm.h"
#include "allocator.h"
#include "exitproc.h"
#include "ffi.h"
#include "findnewline.h"
#include "functions.h"
#include "instructionmapper.h"
#include "instructions.h"
#include "main.h"
#include "print.h"
#include "skipspace.h"
#include "variables.h"
#include "mem.h"
#include "limits.h"
#include "cli.h"

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

	ffivars->map = &vars;
	ffivars->count = 0;
	ffivars->max = limits->variables_max;
	ffivars->namesize = limits->instructions_varnamesize;

	cli(&cliargs);
	char *filedata = getprogram(tempAlloc, limits->misc_maxfilebuffersize, &cliargs);

	char *line;
	unsigned long current_instruction_count = 0;
	Instruction *program = alloc(IRAlloc, limits->allocator_IRAlloc_maxmem);

	/* startup */
	while ((line = findnewline(&filedata)) != NULL) 
	{
		if (current_instruction_count >= limits->instructions_max) 
		{
			print("ERROR: LIMITS: INSTRUCTION CAP REACHED!\n");
			exitproc(1);
		}

		char *p = skipspace(line);
		if (p == NULL) continue;

		program[current_instruction_count] = makeIR(line, limits, tempAlloc, persistAlloc, &instructionMap);
		current_instruction_count++;
	}

	optimizeIR(program, &current_instruction_count, tempAlloc, persistAlloc);

	/* clean the file backing buffer */
	resetAllocator(tempAlloc);

	/* execution */
	unsigned long pc = 0;
	while (pc < current_instruction_count) 
	{
		pc = interpret(&program[pc], program, seed, current_instruction_count, limits, &vars, ffivars, 
			stack, &funcs, pc, persistAlloc, scratchAlloc, tempAlloc);
		resetAllocator(scratchAlloc);
		resetAllocator(tempAlloc);
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

	return 0;
}
