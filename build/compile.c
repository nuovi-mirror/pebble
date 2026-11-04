#include "build.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void compiledeps 
(Config *config, char *srcs, char *objs) 
{
	Buf cmd;
	int result;

	bufinit(&cmd);
	if (config->precompiler[0] != '\0') { bufadd(&cmd, config->precompiler); bufadd(&cmd, " "); }
	bufadd(&cmd, config->compiler);
	bufadd(&cmd, " ");
	bufadd(&cmd, config->cflags);
	bufadd(&cmd, " ");
	bufadd(&cmd, bufstr(&config->usercflags)); /* last, so the user wins */
	bufadd(&cmd, " -c -o ");
	bufadd(&cmd, objs);
	bufadd(&cmd, " ");
	bufadd(&cmd, srcs);

	printf("CC %s\n", srcs);

	result = system(bufstr(&cmd));
	buffree(&cmd);
	if (result != 0) error("compiler failed");
}

void compile 
(Config *config, char **srcs, unsigned int count, char **objs) 
{
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

void linkprog 
(Config *config, char **objs, unsigned int count) 
{
	Buf cmd;
	int result;

	bufinit(&cmd);
	if (config->precompiler[0] != '\0') { bufadd(&cmd, config->precompiler); bufadd(&cmd, " "); }
	bufadd(&cmd, config->compiler);
	bufadd(&cmd, " ");
	bufadd(&cmd, config->ldflags);
	bufadd(&cmd, " ");
	bufadd(&cmd, bufstr(&config->userldflags));
	bufadd(&cmd, " -o ");
	bufshquote(&cmd, config->program);

	for (unsigned int i = 0; i < count; i++) 
	{
		bufadd(&cmd, " ");
		bufadd(&cmd, objs[i]);
	}

	printf("LD %s\n", config->program);

	result = system(bufstr(&cmd));
	buffree(&cmd);
	if (result != 0) error("linker failed");
}

void clean (
Config *config, char **srcs, unsigned int count) 
{
	char objs[1024], deps[1024];

	remove(config->program);

	for (unsigned int i = 0; i < count; i++) 
	{
		sprintf(objs, "%s", srcs[i]);
		objs[strlen(objs) - 1] = 'o'; /* same logic as before */

		sprintf(deps, "%s", objs);
		deps[strlen(deps) - 1] = 'd'; /* same logic as before */

		printf("RM %s\n", objs);
		remove(objs);
		
		printf("RM %s\n", deps);
		remove(deps);
	}
}
