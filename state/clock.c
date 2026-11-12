#include "clockms.h"
#include "clock.h"
#include "main.h"

void timer_init
(Clock *clock)
{
	if (clock == NULL) return;

	*clock = (Clock){0};

	clock->paused = 1;
	clock->waspaused = 1;
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
	if (clock == NULL) return;
	*clock = (Clock){0};
}

void timer_restart
(Clock *clock)
{
	if (clock == NULL) return;
	timer_stop(clock);
	timer_init(clock);
}

void timer_unpause
(Clock *clock)
{
	if (clock == NULL) return;
	clock->paused = 1;
	timer_update(clock);
}

void timer_update
(Clock *clock)
{
	if (clock == NULL) return;
	unsigned long start;
	clock->time = clockms() - clock->start;

	if (clock->waspaused == 0) start = clock->pausedtime;
	else start = clock->start;

	clock->diff = start - clock->time;
}	
