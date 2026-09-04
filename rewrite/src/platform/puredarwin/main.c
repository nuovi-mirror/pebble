#undef NULL
#define NULL ((void *)0)

#include <unistd.h>
#include <stdlib.h>

/* at the time of writting this, PureDarwin has
 * a few unfinished bits, so we need to alias
 * these to avoid linking errors. */

long _read$UNIX2003
(int fd, void *buf, unsigned long nbyte)
{ return read(fd, buf, nbyte); }

int _close$UNIX2003
(int fd)
{ return close(fd); }

double _strtod$UNIX2003
(const char *nptr, char **endptr)
{ return strtod(nptr, endptr); }

/* and now it is just POSIX code */

#include "../platform/use/entry.h"
#include "vm.c"

int main(int argc, char **argv) {
	Args cliargs = initargs(argc, argv);
	Stack *stack = initstack(1024, 1024);
	int ret = vmmain(cliargs, stack);
	freestack(stack);
	return ret;
}
