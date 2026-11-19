#include "main.h"
#include "entry.h"
#include "mem.h"
#include "limits.h"
#include "vm.h"

int main 
(int argc, char **argv)
{
	Args cliargs = initargs(argc, argv);
	Program program = getprogram(12 * 1024 * 1024, &cliargs);
	/* XXX max program file size defined above */

	struct PackedLimits packed = setlimits(&program);
	struct Limits limits = packed.limits;
	struct Memory mem = packed.memory;

	unsigned long long seed = getrandom();
	Stack *stack = initstack((limits.stack_callStack_cap), &mem);

	int ret = vmmain(cliargs, stack, seed, &mem, &limits);
	freestack(stack);
	return ret;
}
