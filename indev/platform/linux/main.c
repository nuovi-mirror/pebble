#include "main.h"
#include "entry.h"
#include "vm.h"
#include "print.h"
#include "exitproc.h"

#include <unistd.h>

int main(int argc, char **argv) {
	char rbuf[64];
	if (getentropy(sizeof(rbuf), rbuf, sizeof(rbuf)) < 0)
	{
		print("ERROR: INIT: CANNOT GENERATE RANDOM DATA WITH getentropy!\n");
		exitproc(1);
	}

	unsigned long long seed = *rbuf;

	Args cliargs = initargs(argc, argv);
	Stack *stack = initstack(1024);
	int ret = vmmain(cliargs, stack, seed);
	freestack(stack);
	return ret;
}
