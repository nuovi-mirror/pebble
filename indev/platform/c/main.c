#include "main.h"
#include "entry.h"
#include "vm.h"

int main(int argc, char **argv) {
	volatile unsigned long long seed = grandom();
	Args cliargs = initargs(argc, argv);
	Stack *stack = initstack(1024);
	int ret = vmmain(cliargs, stack, seed);
	freestack(stack);
	return ret;
}
