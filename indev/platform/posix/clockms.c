#ifndef platform_clockms_
#define platform_clockms_

#define _POSIX_C_SOURCE 200809L /* XXX for Linux */

#include "clockms.h"
#include <time.h>

unsigned long clockms
(void)
{
	struct timespec t;

	clock_gettime(CLOCK_MONOTONIC, &t);
	return (unsigned long)t.tv_sec * 1000UL + (unsigned long)t.tv_nsec / 1000000UL;
}

#endif
