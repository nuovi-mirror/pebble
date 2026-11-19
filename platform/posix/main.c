#include "main.h"
#include "entry.h"
#include "mem.h"
#include "limits.h"
#include "getprogram.h"
#include "cli.h"
#include "vm.h"
#include <stdlib.h> /* XXX for rand(3) */

int main 
(int argc, char **argv)
{
	Args cliargs = initargs(argc, argv);
	cli(&cliargs);
	Program program = getprogram(12 * 1024 * 1024, &cliargs);
	/* XXX max program file size defined above */

	struct PackedLimits packed = setlimits(&program);
	struct Limits limits = packed.limits;
	struct Memory mem = packed.memory;

	unsigned long long seed = rand();
	Stack *stack = initstack((limits.stack_callStack_cap), &mem);

	int ret = vmmain(cliargs, stack, seed, &mem, &limits, &program);
	freestack(stack);
	return ret;
}
