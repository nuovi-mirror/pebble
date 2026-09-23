#include "main.h"
#include "entry.h"
#include "vm.h"

#include <fcntl.h>
#include <unistd.h>

int main(int argc, char **argv) {
	/* UNIX specific seed handling */
	unsigned long long seed = (unsigned long long)rand();

	/* normal code */
	Args cliargs = initargs(argc, argv);
	Stack *stack = initstack(1024);
	int ret = vmmain(cliargs, stack, seed);
	freestack(stack);
	return ret;
}
