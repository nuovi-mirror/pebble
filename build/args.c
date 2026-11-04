#include "build.h"
#include <string.h>

void parseargs (Config *config, char *argument) {
	char *equals = strchr(argument, '=');
	if (equals == NULL) usage();
	*equals++ = '\0';

	if (strcmp(argument, "cc") == 0) config->compiler = equals;
	else if (strcmp(argument, "program") == 0) config->program = equals;
	else if (strcmp(argument, "precc") == 0) config->precompiler = equals;
	else if (strcmp(argument, "build") == 0) config->build = equals;
	else if (strcmp(argument, "platform") == 0) config->platform = equals;
	else if (strcmp(argument, "analyzer") == 0) config->precompiler = equals;
	else if (strcmp(argument, "graphics") == 0) config->graphics = equals;
	else if (strcmp(argument, "terminal") == 0) config->terminal = equals;
	else if (strcmp(argument, "cflags") == 0) { if (!addflags(&config->usercflags, equals)) error("unterminated quote in cflags"); }
	else if (strcmp(argument, "ldflags") == 0) { if (!addflags(&config->userldflags, equals)) error("unterminated quote in ldflags"); }
	else if (strcmp(argument, "proc") == 0) config->proc = equals;
	else if (strcmp(argument, "fs") == 0) config->fs = equals;

	else usage();
}
