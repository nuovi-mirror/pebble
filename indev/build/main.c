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
	char terminal[32];
	char cflags[1024];
	char ldflags[1024];
	char proc[8];
	char fs[8];

	config.compiler = (char *)&compiler[0];
	config.program = (char *)&program[0];
	config.build = (char *)&build[0];
	config.platform = (char *)&platform[0];
	config.precompiler = (char *)&precompiler[0];
	config.graphics = (char *)&graphics[0];
	config.terminal = (char *)&terminal[0];
	config.cflags = (char *)&cflags[0];
	config.ldflags = (char *)&ldflags[0];
	config.proc = (char *)&proc[0];
	config.fs = (char *)&fs[0];

	strcpy(config.compiler, "cc");
	strcpy(config.program, "bin/cpebble");
	strcpy(config.build, "default");
	strcpy(config.platform, "");
	strcpy(config.precompiler, "");
	strcpy(config.graphics, "");
	strcpy(config.terminal, "ansi");
	strcpy(config.cflags, "-Ih -Iffi -std=c99 ");
	strcpy(config.ldflags, "-Wl,--gc-sections ");
	strcpy(config.proc, "");
	strcpy(config.fs, "");

	for (unsigned int i = 1; i < (unsigned int)argc; i++) {
		if (strcmp(argv[i], "clean") == 0) {
			cleanreq = 1;
			continue;
		}

		parseargs(&config, argv[i]);
	}

	detectplatform(&config);

	setflags(&config);
	srccount = mksrclist(&config, src);

	if (cleanreq) {
		clean(&config, src, srccount);
		return 0;
	}

	printf("cflags   : %s\n", config.cflags);
	printf("ldflags  : %s\n", config.ldflags);
	printf("platform : %s\n", config.platform);

	compile(&config, src, srccount, objs);
	linkprog(&config, objs, srccount);

	for (unsigned int i = 0; i < srccount; i++) 
		free(objs[i]);

	return 0;
}
