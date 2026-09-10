#include "main.h"
#include "entry.h"
#include "vm.h"

int main(int argc, char **argv) {
	Args cliargs = initargs(argc, argv);
	Stack *stack = initstack(1024);
	int ret = vmmain(cliargs, stack);
	freestack(stack);
	return ret;
}
