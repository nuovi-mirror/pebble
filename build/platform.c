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

#if defined(__OpenBSD__) /* if OpenBSD */
	return platform_openbsd;
#endif

#ifdef _POSIX_VERSION /* if POSIX */
	return platform_posix;
#endif

	/* else */
	return platform_c;
}

/* XXX puredarwin must be selected manualy since it cannot be distinguished
 * from MacOS here, but this does not really matter anyways since it is
 * depreciated */

void detectplatform
(struct Config *config)
{
	if (config->platform[0] == '\0')
		switch (internal_detectplatform())
		{
			case platform_c: 
				strcpy(config->platform, "c"); 
				strcat(config->cflags, "-Dplatform=\"c\" ");	
				break;
			case platform_posix: 
				strcpy(config->platform, "posix"); 
				strcat(config->cflags, "-Dplatform=\"posix\" ");
				break;
			case platform_openbsd: 
				strcpy(config->platform, "openbsd"); 
				strcat(config->cflags, "-Dplatform=\"openbsd\" ");
				break;
			case platform_puredarwin: 
				strcpy(config->platform, "puredarwin"); 
				strcat(config->cflags, "-Dplatform=\"puredarwin\"");
				break;
		}
}
