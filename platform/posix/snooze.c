#define _POSIX_C_SOURCE 200809L
/* needed for Linux */

#include "snooze.h"
#include <time.h>

unsigned long snooze
(unsigned long ms)
{
	struct timespec time, start, end;
	long sec, nsec;
	unsigned long total;

	time.tv_sec = (time_t)ms / 1000;
	time.tv_nsec = ((time_t)ms % 1000) * 1000000;

	clock_gettime(CLOCK_MONOTONIC, &start);
	nanosleep(&time, NULL);
	clock_gettime(CLOCK_MONOTONIC, &end);

	sec = end.tv_sec - start.tv_sec;
	nsec = end.tv_nsec - start.tv_nsec;

	if (nsec < 0)
	{
		sec--;
		nsec += 1000000000L;
	}

	total = (unsigned long)sec * 1000 + (unsigned long)nsec / 1000000;
	return total;

}
