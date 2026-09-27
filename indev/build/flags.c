#include "build.h"
#include <string.h>

void setflags (Config *config) {
	if (strcmp(config->build, "default") == 0)
		strcat(config->cflags, 
			"-O2 "
			"-flto "
			"-ffunction-sections "
			"-fdata-sections "
		);

	else if (strcmp(config->build, "fast") == 0)
		strcat(config->cflags, 
			"-O3 "
			"-ffunction-sections "
			"-fdata-sections "
			"-flto "
			"-fno-semantic-interposition "
			"-ffast-math "
			"-march=native "
			"-mtune=native "
		);

	else if (strcmp(config->build, "small") == 0)
		strcat(config->cflags, 
			"-Oz "
			"-flto "
			"-ffunction-sections "
			"-fdata-sections "
		);

	else if (strcmp(config->build, "debug") == 0)
		strcat(config->cflags, 
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
			"-Wcast-qual "
			"-Wcast-align "
			"-Wold-style-definition "
			"-Wswitch-enum "
			"-Wvla "
			"-Wdouble-promotion "
			"-Wfloat-equal"
		);

	else
		error("unknown build type");
}
