#include "../platform/use/entry.h"
#include "vm.c"

#define NULL ((void *)0) /* null type */

int main(int argc, char **argv) {
	Args cliargs = initargs(argc, argv);
	Stack *stack = initstack(1024, 1024);
	return vmmain(cliargs, stack);
}

