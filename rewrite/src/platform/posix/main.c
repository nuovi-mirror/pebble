#undef NULL
#define NULL ((void *)0)

#include "../platform/use/entry.h"
#include "vm.c"

int main(int argc, char **argv) {
	Args cliargs = initargs(argc, argv);
	Stack *stack = initstack(1024, 1024);
	return vmmain(cliargs, stack);
	freestack(stack);
}
