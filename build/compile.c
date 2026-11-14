#include "build.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef nomulti
#ifdef __unix__
#include <unistd.h>
#include <pthread.h>
#endif
#endif

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
	bufadd(&cmd, config->ldflags);
	bufadd(&cmd, " ");
	bufadd(&cmd, bufstr(&config->usercflags)); /* last, so the user wins */
	bufadd(&cmd, " -c -o ");
	bufadd(&cmd, objs);
	bufadd(&cmd, " ");
	bufadd(&cmd, srcs);

	printf("cc %s\n", srcs);
	result = system(bufstr(&cmd));
	buffree(&cmd);
	if (result != 0) error("compiler failed");
}

void *compiledeps_packed
(void *arg)
{
	PackedDeps *deps = (PackedDeps *)arg;
	compiledeps(deps->config, deps->srcs, deps->objs);
	free(deps);
	return NULL;
}

void compile 
(Config *config, char **srcs, unsigned int count, char **objs) 
{
	char obj[1024];
#ifndef nomulti
#ifdef __unix__
	pthread_t threads[count];
#endif
#endif

	for (unsigned int i = 0; i < count; i++) {
		sprintf(obj, "%s", srcs[i]);
		obj[strlen(obj) - 1 ] = 'o'; /* replace the 'c' in '.c' with an 'o' */
		objs[i] = malloc(strlen(obj) + 1);

		if (objs[i] == NULL) error("out of memory");

		strcpy(objs[i], obj);

#ifndef nomulti
#ifdef __unix__
		PackedDeps *deps = malloc(sizeof(PackedDeps));
		if (deps == NULL) error("Out Of Memory!");

		deps->config = config;
		deps->srcs = srcs[i];
		deps->objs = objs[i];

		int err = pthread_create(&threads[i], NULL, compiledeps_packed, (void *)deps);
		if (err != 0) {
			free(deps);
			error("Cannot create thread!");
		}
#endif
#else
		compiledeps(config, srcs[i], objs[i]);
#endif
	}

#ifndef nomulti
#ifdef __unix__
	for (unsigned int i = 0; i < count; i++) {
		int err =pthread_join(threads[i], NULL);
		if (err != 0) error("Failed to join threads!");
	}
#endif
#endif

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
	bufadd(&cmd, config->cflags);
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

	printf("ld %s\n", config->program);

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

		printf("rm %s\n", objs);
		remove(objs);
		
		printf("rm %s\n", deps);
		remove(deps);
	}
}
