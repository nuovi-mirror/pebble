#include "build.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int main (int argc, char **argv) {
	struct Config config;
	char *src[1024], *objs[1024];
	unsigned int srccount, cleanreq = 0;

	char compiler[32];
	char program[32];
	char build[32];
	char platform[32];
	char precompiler[32];
	char graphics[32];
	char cflags[1024];
	char ldflags[1024];

	config.compiler = (char *)&compiler;
	config.program = (char *)&program;
	config.build = (char *)&build;
	config.platform = (char *)&platform;
	config.precompiler = (char *)&precompiler;
	config.graphics = (char *)&graphics;
	config.cflags = (char *)&cflags;
	config.ldflags = (char *)&ldflags;

	strcpy(config.compiler, "cc");
	strcpy(config.program, "bin/vm");
	strcpy(config.build, "default");
	strcpy(config.platform, "c");
	strcpy(config.precompiler, "");
	strcpy(config.graphics, "");
	strcpy(config.cflags, "-Ih -Iffi -std=c99 ");
	strcpy(config.ldflags, "-Wl,--gc-sections ");

	for (unsigned int i = 1; i < (unsigned int)argc; i++) {
		if (strcmp(argv[i], "clean") == 0) {
			cleanreq = 1;
			continue;
		}

		parseargs(&config, argv[i]);
	}

	setflags(&config);
	srccount = mksrclist(&config, src);

	if (cleanreq) {
		clean(&config, src, srccount);
		return 0;
	}

	/* XXX debug
	printf("cflags  : %s\n", config.cflags);
	printf("ldflags : %s\n", config.ldflags);
	*/

	compile(&config, src, srccount, objs);
	link(&config, objs, srccount);

	for (unsigned int i = 0; i < srccount; i++) 
		free(objs[i]);

	return 0;
}
