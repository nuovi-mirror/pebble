#include "main.h"
#include "entry.h"
#include "mem.h"
#include "limits.h"
#include "vm.h"

int main 
(int argc, char **argv)
{
	struct PackedLimits packed = setlimits();
	struct Limits limits = packed.limits;
	struct Memory mem = packed.memory;

	unsigned long long seed = getrandom();

	Args cliargs = initargs(argc, argv);
	Stack *stack = initstack((limits.stack_callStack_frameSize *limits.stack_callStack_cap), &mem);

	int ret = vmmain(cliargs, stack, seed, &mem, &limits);
	freestack(stack);
	return ret;
}
