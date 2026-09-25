#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct Config {
	char *cc;
	char *program;
	char *build;
	char *target;
	char *analyzer;
	char *graphics;
	char *cflags;
	char *ldflags;
};

static char *vm_srcs[] = {
	"allocator/allocator.c",
	"allocator/mem.c",
	
	"state/evaluator.c",
	"state/expressions.c",
	"state/functions.c",
	"state/hashmap.c",
	"state/instructionmapper.c",
	"state/instructions.c",
	"state/values.c",
	"state/variables.c",

	"vm/vm.c",

	NULL
};

static char *platform_freestand_srcs[] = {
	"platform/freestand/cmpstr.c",
	"platform/freestand/cmpstrn.c",
	"platform/freestand/copystr.c",
	"platform/freestand/findnewline.c",
	"platform/freestand/getnstrlen.c",
	"platform/freestand/getstrlen.c",
	"platform/freestand/skipspace.c",
	"platform/freestand/copymem.c",
	"platform/freestand/strsplit.c",
	"platform/freestand/inputl.c",

	NULL
};

static char *platform_c_srcs[] = {
	"platform/c/entry.c",
	"platform/c/exitproc.c",
	"platform/c/setmem.c",
	"platform/c/print.c",
	"platform/c/readfile.c",
	"platform/c/str2ul.c",
	"platform/c/snprint.c",

	NULL
};

static char *platform_posix_srcs[] = {
	"platform/posix/inputn.c",

	NULL
};

static char *ffi_base_srcs[] = {
	"ffi/ffi.c",
	"ffi/escapes.c",

	NULL
};

static char *ffi_c_srcs[] = {
	"ffi/platform/c/test/hello.c",
	"ffi/platform/c/test/getseed.c",
	"ffi/platform/c/std/io/print.c",
	"ffi/platform/c/std/io/printLn.c",

	NULL
};

static char *ffi_sdl2_srcs[] = {
	"ffi/platform/sdl2/gs/2d/window/create.c",

	NULL
};

static void error (char *msg) {
	fprintf(stderr, "Error: %s\n", msg);
	exit(1);
}

static void usage (void) {
	fprintf(stderr, 
			"usage: build [clean] [var=value]\n"
			"\n"
			"vars:\n"
			"  cc=compiler\n"
			"  program=program\n"
			"  build=default|debug|fast|small\n"
			"  platform=freestand|c|posix|openbsd|puredarwin\n"
			"  analyzer=analyzer\n"
			"  graphics=graphics\n"
			"  cflags=cflags\n"
			"  ldflags=ldflags\n"
		);

	exit(0);
}


static void parseargs (struct Config *config, char *argument) {
	char *equals = strchr (argument, '=');

	if (equals == NULL) usage();

	*equals = '\0';
	equals++;

	if (strcmp (argument, "cc") == 0) config->cc = equals;
	else if (strcmp (argument, "program") == 0) config->program = equals;
	else if (strcmp (argument, "build") == 0) config->build = equals;
	else if (strcmp (argument, "platform") == 0) config->target = equals;
	else if (strcmp (argument, "analyzer") == 0) config->analyzer = equals;
	else if (strcmp (argument, "graphics") == 0) config->graphics = equals;
	else if (strcmp (argument, "cflags") == 0) config->cflags = equals;
	else if (strcmp (argument, "ldflags") == 0) config->ldflags = equals;
	else usage();
}

static char *main_srcs (char *target) {
	if (strcmp(target, "freestand") == 0) return "platform/freestand/main.c";
	if (strcmp(target, "c") == 0) return "platform/c/main.c";
	if (strcmp(target, "posix") == 0) return "platform/posix/main.c";
	if (strcmp(target, "openbsd") == 0) return "platform/openbsd/main.c";
	if (strcmp(target, "puredarwin") == 0) return "platform/puredarwin/main.c";

	error("unknown platform");
	return NULL;
}

static void addsrcs (char **srcs, char **all, int *count) {
	for (int i = 0; srcs[i] != NULL; i++) {
		all[*count] = srcs[i];
		(*count)++;
	}
}

static int mksrclist (struct Config *config, char **srcs) {
	int count = 0;

	addsrcs(vm_srcs, srcs, &count);

	if (strcmp(config->target, "freestand") == 0) { 
		addsrcs(platform_freestand_srcs, srcs, &count);
	} else if (strcmp(config->target, "c") == 0) {
		addsrcs(platform_c_srcs, srcs, &count);
		addsrcs(platform_freestand_srcs, srcs, &count);
	} else if (
		strcmp(config->target, "posix") == 0 ||
		strcmp(config->target, "openbsd") == 0 ||
		strcmp(config->target, "puredarwin") == 0
	) {

		addsrcs(platform_posix_srcs, srcs, &count);
		addsrcs(platform_c_srcs, srcs, &count);
		addsrcs(platform_freestand_srcs, srcs, &count);
	} else error("unknown target");

	addsrcs(ffi_base_srcs, srcs, &count);

	if (
		strcmp(config->target, "c") == 0 ||
		strcmp(config->target, "posix") == 0 ||
		strcmp(config->target, "openbsd") == 0 ||
		strcmp(config->target, "puredarwin") == 0
	) addsrcs(ffi_c_srcs, srcs, &count);

	if (strcmp(config->graphics, "stdl2") == 0) 
		addsrcs(ffi_sdl2_srcs, srcs, &count);

	srcs[count++] = main_srcs(config->target);
	return count;
}

static void compilesrcs (struct Config *config, char *srcs, char *objs, char *cppflags,
	char *cflags
) {
	char cmd[4096];
	
	sprintf(cmd, "%s%s %s %s %s %s -MMD -MP -c -o %s %s", 
		config->analyzer, 
		config->analyzer[0] != '\0' ? " " : "",
		config->cc,
		cppflags,
		cflags,
		config->cflags,
		objs,
		srcs
	);

	fprintf(stderr, "CC %s\n", srcs);

	int result = system(cmd);

	if (result != 0) error("compiler failed");
}

static void compileall (struct Config *config, char **srcs, int count, char **objs,
	char *cppflags, char *cflags
) {
	char obj[1024];

	for (int i = 0; i < count; i++) {
		sprintf(obj, "%s", srcs[i]);
		obj[strlen(obj) - 1] = 'o';
		objs[i] = malloc(strlen(obj) + 1);

		if (objs[i] == NULL)
			error("Out of memory");

		strcpy(objs[i], obj);
		compilesrcs(config, srcs[i], objs[i], cppflags, cflags);
	}
}

static void linkprogram (struct Config *config, char **objs, int count) {
	char cmd[16386];

	sprintf(cmd, "%s%s %s %s -o %s",
			config->analyzer,
			config->analyzer[0] != '\0' ? " " : "",
			config->cc,
			config->ldflags,
			config->program
	);

	unsigned long offset = strlen(cmd);

	for (int i = 0; i < count; i++) {
		sprintf(cmd + offset, " %s", objs[i]);
		offset = strlen(cmd);
	}

	fprintf(stderr, "LD %s\n", config->program);
	if (system(cmd) != 0)
		error("linker failed");
}

static void clean (char **srcs, int count, char *program) {
	char objs[4096];
	char deps[1024];

	remove(program);

	for (int i = 0; i < count; i++) {
		sprintf(objs, "%s", srcs[i]);
		objs[strlen(objs) - 1] = 'o';
		sprintf(deps, "%s", objs);
		deps[strlen(deps) - 1] = 'd';
		remove(objs);
		remove(deps);
	}
}

static char *buildflags (char *build) {
	if (strcmp(build, "default") == 0)
		return
			"-std=c99 "
			"-O2 "
			"-flto "
			"-ffunction-sections "
			"-fdata-sections";
	if (strcmp(build, "debug") == 0)
		return
			"-std=c99 "
			"-g "
			"-O0 "
			"-Wall "
			"-Wextra "
			"-Wpedantic "
			"-Wshadow "
			"-Wconversion "
			"-Wsign-conversion "
			"-Wformat=2 "
			"-Wundef "
			"-Wcast-equal "
			"-Wcase-align "
			"-Wold-style-definitions "
			"-Wswitch-enum "
			"-Wvla "
			"-Wdouble-promotion "
			"-Wfloat-equal";

	if (strcmp(build, "small") == 0)
		return
			"-std=c99 "
			"-Oz "
			"-flto "
			"-ffunction-sections "
			"-fdata-sections";

	if (strcmp(build, "fast") == 0)
		return
			"-std=c99 "
			"-O3 "
			"-ffunction-sections "
			"-fdata-sections "
			"-flto "
			"-fno-semantic-interposition "
			"-ffast-math "
			"-march=native "
			"-mtune=native";

	error("unknown build type");
	return NULL;
}



int main(int argc, char **argv) {
	struct Config config;
	char *srcs[256], *objs[256];
	char *cppflags, *cflags;
	int srcscount, cleanreq;

	config.cc = "cc";
	config.program = "bin/vm";
	config.build = "default";
	config.target = "c";
	config.analyzer = "";
	config.graphics = "";
	config.cflags = "";
	config.ldflags = "-Wl,--gc-sections";
	cleanreq = 0;

	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "clean") == 0) {
			cleanreq = 1;
			continue;
		}

		parseargs(&config, argv[i]);
	}

	cppflags = "-Ih -Iffi";
	cflags = buildflags(config.build);
	srcscount = mksrclist(&config, srcs);

	if (cleanreq) {
		clean(srcs, srcscount, config.program);
		return 0;
	}

	compileall(&config, srcs, srcscount, objs, cppflags, cflags);
	linkprogram(&config, objs, srcscount);

	for (int i = 0; i < srcscount; i++)
		free(objs[i]);

	return 0;
}	
