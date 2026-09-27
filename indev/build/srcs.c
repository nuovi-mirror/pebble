#include "build.h"
#include <stdio.h>
#include <string.h>

const char *getmain (Config *config) {
	if (strcmp(config->platform, "freestand") == 0) return "platform/freestand/main.c";
	if (strcmp(config->platform, "c") == 0) return "platform/c/main.c";
	if (strcmp(config->platform, "posix") == 0) return "platform/posix/main.c";

	if (strcmp(config->platform, "openbsd") == 0) return "platform/openbsd/main.c";
	if (strcmp(config->platform, "puredarwin") == 0) return "platform/puredarwin/main.c";

	error("unknown platform");
	return NULL; /* never reached */
}

void addsrcs (const char **srcs, char **all, unsigned int *count) {
	for (unsigned int i = 0; srcs[i] != NULL; i++) {
		all[*count] = srcs[i];
		(*count)++;
	}
}

unsigned int mksrclist (Config *config, char **srcs) {
	unsigned int count = 0;

	addsrcs(vm_srcs, srcs, &count);

	if (strcmp(config->platform, "freestand") == 0) 
		addsrcs(platform_freestand_srcs, srcs, &count);
	else if (strcmp(config->platform, "c") == 0) {
		addsrcs(platform_c_srcs, srcs, &count);
		addsrcs(platform_freestand_srcs, srcs, &count);
	} else if (strcmp(config->platform, "posix") == 0 ||
		 strcmp(config->platform, "openbsd") == 0 ||
		 strcmp(config->platform, "puredarwin") == 0)
	{
		addsrcs(platform_posix_srcs, srcs, &count);
		addsrcs(platform_c_srcs, srcs, &count);
		addsrcs(platform_freestand_srcs, srcs, &count);
	}

	addsrcs(ffi_base_srcs, srcs, &count);
	
	if (
		strcmp(config->platform, "c") == 0 ||
		strcmp(config->platform, "posix") == 0 ||
		strcmp(config->platform, "openbsd") == 0 ||
		strcmp(config->platform, "puredarwin") == 0)
	{
		addsrcs(ffi_c_srcs, srcs, &count);
	}


	if (strcmp(config->graphics, "sdl2") == 0) {
		addsrcs(ffi_sdl2_srcs, srcs, &count);

		char sdl2_ldflags[1024];
		char sdl2_cflags[1024];

		/* parse sdl2.conf 
		 * line 1 - sdl2 linker flags 
		 * line 2 - sdl2 compiler flags */

		FILE *file = fopen("sdl2.conf", "r");
		if (file == NULL) error("cannot open sdl2.conf");

		if (fgets(sdl2_ldflags, 1024, file) == NULL) {
			fclose(file);
			error("cannot read sdl2.conf, line one");
		}

		if (fgets(sdl2_cflags, 1024, file) == NULL) {
			fclose(file);
			error("cannot read sdl2.conf, line two");
		}

		fclose(file);

		sdl2_ldflags[strcspn(sdl2_ldflags, "\n")] = '\0';
		sdl2_cflags[strcspn(sdl2_cflags, "\n")] = '\0';

		strcat(config->ldflags, sdl2_ldflags);
		strcat(config->cflags, sdl2_cflags);

	}

	srcs[count++] = getmain(config);
	return count;
}
