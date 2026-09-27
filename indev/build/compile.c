#include "build.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void compiledeps (Config *config, char *srcs, char *objs) {
	char cmd[4096]; /* command buffer */

	sprintf(cmd, "%s%s %s %s -MMD -MP -c -o %s %s", 
		config->precompiler, 
		config->precompiler[0] != '\0' ? " " : "",
		config->compiler,
		config->cflags,
		objs,
		srcs
	);

	printf("CC %s\n", srcs);

	int result = system(cmd);
	if (result != 0) error("compiler failed");
}

void compile (Config *config, char **srcs, unsigned int count, char **objs) {
	char obj[1024];

	for (unsigned int i = 0; i < count; i++) {
		sprintf(obj, "%s", srcs[i]);
		obj[strlen(obj) - 1 ] = 'o'; /* replace the 'c' in '.c' with an 'o' */
		objs[i] = malloc(strlen(obj) + 1);

		if (objs[i] == NULL) error("out of memory");

		strcpy(objs[i], obj);
		compiledeps(config, srcs[i], objs[i]);
	}
}

/* please dont include unistd.h */
void link (Config *config, char **objs, unsigned int count) {
	char cmd[16386]; /* command buffer */

	sprintf(cmd, "%s%s %s %s -o %s", 
		config->precompiler, 
		config->precompiler[0] != '\0' ? " " : "",
		config->compiler,
		config->ldflags,
		config->program
	);

	unsigned long offset = strlen(cmd);

	for (unsigned int i = 0; i < count; i++) {
		sprintf(cmd + offset, " %s", objs[i]);
		offset = strlen(cmd);
	}

	printf("LD: %s\n", config->program);

	if (system(cmd) != 0) error("linker failed");
}

void clean (Config *config, char **srcs, unsigned int count) {
	char objs[1024], deps[1024];

	remove(config->program);

	for (unsigned int i = 0; i < count; i++) {
		sprintf(objs, "%s", srcs[i]);
		objs[strlen(objs) - 1] = 'o'; /* same logic as before */

		sprintf(deps, "%s", objs);
		deps[strlen(deps) - 1] = 'd'; /* same logic as before */

		remove(objs);
		remove(deps);
	}
}
