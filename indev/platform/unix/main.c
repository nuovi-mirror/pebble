#include "main.h"
#include "entry.h"
#include "vm.h"
#include "print.h"
#include "exitproc.h"

#include <fcntl.h>
#include <unistd.h>

int main(int argc, char **argv) {
	/* UNIX specific seed handling */
	unsigned long long seed;
	long got = 0;
	
	int fd = open("/dev/urandom", O_RDONLY);
	if (fd < 0)
	{
		print("ERROR: INIT: CANNOT OPEN /dev/urandom FOR RANDOM SEED!\n");
		exitproc(1);
	}

	while (got < sizeof(seed)) {
		long n = read(fd, (char *)&seed + got, sizeof(seed) - got);

		if (n <= 0)
		{
			print("ERROR: INIT: CANNOT READ FROM /dev/urandom TO GET RANDOM SEED!\n");
			exitproc(1);
		}

		got += n;
	}

	close(fd);

	/* normal code */
	Args cliargs = initargs(argc, argv);
	Stack *stack = initstack(1024);
	int ret = vmmain(cliargs, stack, seed);
	freestack(stack);
	return ret;
}
