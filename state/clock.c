#include "clockms.h"
#include "clock.h"
#include "main.h"

void timer_init
(Clock *clock)
{
	if (clock == NULL) return;

	clock->paused = 1;
	clock->waspaused = 1;
	clock->pausedtime = 0;
	clock->start = 0;
	clock->time = 0;
	clock->end = 0;
	clock->diff = 0;
}

void timer_start
(Clock *clock)
{
	if (clock == NULL) return;
	clock->start = clockms();
}

void timer_pause
(Clock *clock)
{
	if (clock == NULL) return;
	clock->paused = 0;
	clock->waspaused = 0;
	clock->pausedtime = clockms();
}

void timer_stop
(Clock *clock)
{
	/* does nothing since the caller gives the clock struct */
	return;
}

void timer_unpause
(Clock *clock)
{
	if (clock == NULL) return;
	clock->paused = 1;
}

void timer_update
(Clock *clock)
{
	if (clock == NULL) return;
	clock->time = clockms();
	if (clock->waspaused == 0) clock->diff = clock->pausedtime - clock->curr;
	else clock->diff = clock->start - clock->curr;
}	
