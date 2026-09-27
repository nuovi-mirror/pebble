#include "build.h"
#include <string.h>

void parseargs (Config *config, char *argument) {
	char *equals = strchr(argument, '=');
	if (equals == NULL) usage();
	*equals++ = '\0';

	if (strcmp(argument, "cc") == 0) config->compiler = equals;
	else if (strcmp(argument, "program") == 0) config->program = equals;
	else if (strcmp(argument, "build") == 0) config->build = equals;
	else if (strcmp(argument, "platform") == 0) config->platform = equals;
	else if (strcmp(argument, "analyzer") == 0) config->precompiler = equals;
	else if (strcmp(argument, "graphics") == 0) config->graphics = equals;
	else if (strcmp(argument, "cflags") == 0) config->cflags = equals;
	else if (strcmp(argument, "ldflags") == 0) config->ldflags = equals;

	else usage();
}
