#include "main.h"
#include "entry.h"
#include "mem.h"
#include "limits.h"
#include "vm.h"
#include "lalloc.h" /* initial backings */
#include "print.h" /* XXX error handling */
#include "appendstr.h" /* XXX for pledge(2) init */
#include <unistd.h> /* XXX for pledge(2) and unveil(2) */
#include <stdlib.h> /* XXX for arc4random(3) */

int main 
(int argc, char **argv)
{
	/* XXX unveil(2) code */
#ifdef fs
	print("VM: OPENBSD: Cannot use unveil(2): Arbitrary File I/O Enabled\n");
#else
	if (argc >= 2) 
	{
		print("VM: OPENBSD: Using unveil(2) on ");
		print(argv[1]);
		print("\n");

		if (unveil(argv[1], "r") == -1) 
			error(errfatal, errcodeplatformspecific, "Failed to run unveil(2)!");
	}

	print("VM: OPENBSD: Locking unveil(2)\n");
	unveil(NULL, NULL);
#endif

	print("VM: OPENBSD: Setting up pledge(2) restrictons...\n");

	char bbuff[128] = { 0 };
	char *buff = &bbuff[0];
	appendstr("rpath tty stdio", buff);

#ifdef proc
	print("VM: OPENBSD: Subrocess management support enabled\n");
	appendstr(" proc exec", buff);
#endif

#ifdef fs
	print("VM: OPENBSD: File manipulation support enabled\n");
	appendstr(" wpath cpath", buff);
#endif

	/* XXX pledge(2) code */
	print("VM: OPENBSD: Locking down to '");
	print(buff);
	print("' via pledge(2)\n");

	if (pledge(buff, NULL) == -1) error(errfatal, errcodeplatformspecific, "Failed to run pledge(2)!");

	struct PackedLimits packed = setlimits();
	struct Limits limits = packed.limits;
	struct Memory mem = packed.memory;

	/* XXX replaced rand(3) with arc4random(3) */
	unsigned long long seed;
	arc4random_buf(&seed, sizeof(seed));

	Args cliargs = initargs(argc, argv);
	Stack *stack = initstack((limits.stack_callStack_frameSize *limits.stack_callStack_cap), &mem);

	int ret = vmmain(cliargs, stack, seed, &mem, &limits);
	freestack(stack);
	return ret;
}
