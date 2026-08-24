/* POSIX-specific program entry point */

#include <stddef.h>

/* this is responsible for doing three things */
/*   - getting command-line arguments for the platform */
/*   - formatting the command-line arguments for the VM */
/*   - calling the main VM entry at vmmain */

typedef struct Args {
	size_t count;
	char **values;
} Args;

#include "vm.c" /* include after so the struct is visible */

/* VM arguments follow a POSIX-esque structure */
/*   - Args struct */
/*     - size_t count = number of args */
/*     - char **values = array of arguments */
/* the last argument should be a null character */

int main(int argc, char **argv) {
	Args cliargs; /* init argument struct */

	cliargs.count = (size_t)argc; /* set the count */
	cliargs.values - argv; /* set the arguments */

	return vmmain(cliargs); /* call main VM entry point */

	/* and our job here is done */
}
