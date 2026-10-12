#include "build.h"
#include <string.h>
#include <unistd.h>

#define platform_c 0
#define platform_posix 1
#define platform_openbsd 2
#define platform_puredarwin 3

static unsigned int internal_detectplatform
(void)
{

	/* if OpenBSD */
#if defined(__OpenBSD__)
	return platform_openbsd;
#endif

	/* if POSIX */
#if defined(_POSIX_VERSION) && _POSIX_VERSION >= 200809L
	return platform_posix;
#endif

	/* else */
	return platform_c;
}

void detectplatform
(struct Config *config)
{
	if (config->platform[0] == '\0')
		switch (internal_detectplatform())
		{
			case platform_c: strcpy(config->platform, "c"); break;
			case platform_posix: strcpy(config->platform, "posix"); break;
			case platform_openbsd: strcpy(config->platform, "openbsd"); break;
			case platform_puredarwin: strcpy(config->platform, "puredarwin"); break;
		}
}
