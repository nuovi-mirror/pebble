#include "build.h"
#include <string.h>

void setflags (Config *config) {
	if (strcmp(config->build, "default") == 0)
	{
		strcat(config->cflags, 
			"-O2 "
			"-ffunction-sections "
			"-fdata-sections "
		);

		strcat(config->ldflags, "-flto ");
	}

	else if (strcmp(config->build, "fast") == 0)
	{
		strcat(config->cflags, 
			"-O3 "
			"-ffunction-sections "
			"-fdata-sections "
			"-fno-semantic-interposition "
			"-ffast-math "
			"-march=native "
			"-mtune=native "
		);

		strcat(config->ldflags, "-flto ");
	}

	else if (strcmp(config->build, "xray") == 0) {
		strcat(config->cflags, 
			"-O2 "
			"-g "
			"-fxray-instrument "
			"-fxray-instruction-threshold=1 "
			"-fxray-link-deps "
		);

		strcat(config->ldflags, "-fxray-instrument");
	}


	else if (strcmp(config->build, "small") == 0)
	{
		strcat(config->cflags, 
			"-Oz "
			"-ffunction-sections "
			"-fdata-sections "
		);

		strcat(config->ldflags, "-flto ");
	}

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
			"-fno-omit-frame-pointer "
			"-fno-optimize-sibling-calls "
			"-Wformat=2 "
			"-Wundef "
			"-Wcast-qual "
			"-Wcast-align "
			"-Wold-style-definition "
			"-Wswitch-enum "
			"-Wvla "
			"-Wdouble-promotion "
			"-Wfloat-equal "
		);

	else
		error("unknown build type");
}
