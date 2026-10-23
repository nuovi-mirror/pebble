#include "clockms.h"

#define _POSIX_C_SOURCE 200809L
#include <time.h>

unsigned void clockms
(void)
{
	struct timespec t;

	clock_gettime(CLOCK_MONOTIC, &t);
	return (unsigned long)t.tv_sec * 1000UL + (unsigned long)tv.nsec / 1000000UL;
}
