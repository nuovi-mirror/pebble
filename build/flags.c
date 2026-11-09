#include "build.h"
#include <string.h>

void setflags (Config *config) {
	if (strcmp(config->build, "default") == 0)
	{
		strcat(config->cflags, 
			"-O2 "
			"-ffunction-sections "
			"-fdata-sections "
			"-Dbuild=\"default\" "
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
			"-Dbuild=\"fast\" "
		);

		strcat(config->ldflags, "-flto -Dnozeromem ");
	}

	else if (strcmp(config->build, "xray") == 0) {
		strcat(config->cflags, 
			"-O2 "
			"-g "
			"-fxray-instrument "
			"-fxray-instruction-threshold=1 "
			"-fxray-link-deps "
			"-Dbuild=\"xray\" "
		);

		strcat(config->ldflags, "-fxray-instrument");
	}


	else if (strcmp(config->build, "small") == 0)
	{
		strcat(config->cflags, 
			"-Oz "
			"-ffunction-sections "
			"-fdata-sections "
			"-Dbuild=\"small\" "
		);

		strcat(config->ldflags, "-flto ");
	}

	else if (strcmp(config->build, "bad") == 0)
		strcat(config->cflags,
			"-O0 "
			"-Wall "
			"-Wextra "
			"-Wuninitialized "
			"-Dbuild=\"bad\" "
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
			"-Ddebug "
			"-Dbuild=\"debug\" "
		);

	else
		error("unknown build type");
}
