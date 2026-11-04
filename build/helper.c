#include "build.h"
#include <stdio.h>
#include <stdlib.h>

void error (const char *msg) {
	fprintf(stderr, "Error: %s\n", msg);
	exit(1);	
}

void usage (void) {
	printf(
		"usage: build [clean] [var=value]\n\n"
		"vars:\n"
		"  cc=[compiler]\n"
		"  program=[program output binary]\n"
		"  build=[default|debug|fast|small]\n"
		"  platform=[c|posix|openbsd|puredarwin]\n"
		"  precc=[pre-compiler]\n"
		"  cflags=[compiler flags]\n"
		"  ldflags=[linker flags]\n"
	);

	exit(1);
}
