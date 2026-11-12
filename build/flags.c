#include "build.h"
#include <string.h>

void setflags (Config *config) {
	if (strcmp(config->build, "default") == 0)
	{
		strcat(config->cflags, 
			"-O2 "
			"-ffunction-sections "
			"-fdata-sections "
			"-Dbuild_default "
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
			"-Dbuild_fast "
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
			"-Dbuild_xray "
		);

		strcat(config->ldflags, "-fxray-instrument");
	}


	else if (strcmp(config->build, "small") == 0)
	{
		strcat(config->cflags, 
			"-Oz "
			"-ffunction-sections "
			"-fdata-sections "
			"-Dbuild_small "
		);

		strcat(config->ldflags, "-flto ");
	}

	else if (strcmp(config->build, "bad") == 0)
		strcat(config->cflags,
			"-O0 "
			"-Wall "
			"-Wextra "
			"-Wuninitialized "
			"-Dbuild_bad "
		);

	else if (strcmp(config->build, "debug") == 0)
		strcat(config->cflags, 
			"-g "
			"-O0 "
			"-Wall "
			"-Wextra "
			"-Wshadow "
			"-fno-omit-frame-pointer "
			"-fno-optimize-sibling-calls "
			"-Wformat=2 "
			"-Wold-style-definition "
			"-Wvla "
			"-Wdouble-promotion "
			"-Wfloat-equal "
			"-Ddebug "
			"-Dbuild_debug "
			"-Wno-error=switch "
			"-Wno-error=unused-parameter "
			"-Wno-error=missing-field-initializers "
			"-Wno-error=missing-format-attribute "
			"-Wno-error=sign-compare "
		);

	else
		error("unknown build type");
}
