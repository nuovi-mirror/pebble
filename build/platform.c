#include "build.h"
#include <string.h>
#include <unistd.h>

#define platform_c 0
#define platform_posix 1
#define platform_openbsd 2
#define platform_puredarwin 3

static volatile void setplatform_c
(struct Config *config)
{
	strcpy(config->platform, "c"); 
	strcat(config->cflags, "-Dplatform_c ");	
}

static volatile void setplatform_posix
(struct Config *config)
{
	strcpy(config->platform, "posix"); 
	strcat(config->cflags, "-Dplatform_posix ");
}

static volatile void setplatform_openbsd
(struct Config *config)
{			
	strcpy(config->platform, "openbsd"); 
	strcat(config->cflags, "-Dplatform_openbsd ");
}

static volatile void setplatform_puredarwin
(struct Config *config)
{
	strcpy(config->platform, "puredarwin"); 
	strcat(config->cflags, "-Dplatform_puredarwin");
}

static volatile unsigned int internal_detectplatform
(void)
{
#if defined(__OpenBSD__) /* if OpenBSD */
	return platform_openbsd;
#endif

#ifdef _POSIX_VERSION /* if POSIX */
	return platform_posix;
#else
	return platform_c;
#endif
}

/* XXX puredarwin must be selected manualy since it cannot be distinguished
 * from MacOS here, but this does not really matter anyways since it is
 * depreciated */

void detectplatform
(struct Config *config)
{
	volatile unsigned int platformt;
	if (config->platform[0] == '\0') platformt = internal_detectplatform();	
	else
	{
		if (strcmp(config->platform, "c") == 0) platformt = platform_c;
		else if (strcmp(config->platform, "posix") == 0) platformt = platform_posix;
		else if (strcmp(config->platform, "openbsd") == 0) platformt = platform_openbsd;
		else if (strcmp(config->platform, "puredarwin") == 0) platformt = platform_puredarwin;
	}

	switch (platformt)
	{
		case platform_c: setplatform_c(config); break;
		case platform_posix: setplatform_posix(config); break;
		case platform_openbsd: setplatform_openbsd(config); break;
		case platform_puredarwin: setplatform_puredarwin(config); break;
	}
}
