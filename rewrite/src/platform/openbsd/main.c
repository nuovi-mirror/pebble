#undef NULL
#define NULL ((void *)0) /* null type */

#include "../platform/use/entry.h"
#include "../platform/use/print.h"
#include "../platform/use/exitproc.h"
#include "vm.c"

#include <unistd.h> /* needed for the two magical calls */

/* literally just the POSIX one but with 
 * pledge and unveil calls added */

int main(int argc, char **argv) {
	/* the first argument is generally a file containing bytecode to 
	 * execute, so allow us to read that file if it exists */

	if (argc < 2) { /* no arguments were given */
		unveil(NULL, NULL); /* no access */
	} else {
		if (access(argv[1], F_OK) == 0) { /* file exists */
			/* RO to the file containing the bytecode to execute, nothing else */
			if (unveil(argv[1], "r") == -1) {
				print("ERROR: INIT: OPENBSD: UNVEIL FAILED!\n");
				exitproc(1);
			}
		}

		/* if does not exist, no file I/O */
		if (unveil(NULL, NULL) == -1) {
			print("ERROR: INIT: OPENBSD: UNVEIL FAILED!\n");
			exitproc(1);
		}
	}


	/* memory management, teletype I/O, subprocess management */
	if (pledge("stdio rpath tty proc", NULL) == -1) {
		print("ERROR: INIT: OPENBSD: PLEDGE FAILED!\n");
		exitproc(1);
	}

	/* now we can do init */

	Args cliargs = initargs(argc, argv);
	Stack *stack = initstack(1024, 1024);
	
	return vmmain(cliargs, stack);
}

