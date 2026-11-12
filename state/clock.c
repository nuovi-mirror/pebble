#include "clockms.h"
#include "clock.h"
#include "main.h"

void timer_init
(Clock *clock)
{
	if (clock == NULL) return;

	*clock = (Clock){0};

	clock->paused = false;
	clock->waspaused = false;
}

void timer_start
(Clock *clock)
{
	if (clock == NULL) return;
	clock->start = clockms();
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
	timer_start(clock);
}

void timer_pause
(Clock *clock)
{
	if (clock == NULL) return;
	clock->paused = true;
	clock->waspaused = true;
	clock->pausedtime = clockms();
}

void timer_unpause
(Clock *clock)
{
	if (clock == NULL) return;
	clock->paused = false;
	timer_update(clock);
}

void timer_update
(Clock *clock)
{
	if (clock == NULL) return;
	unsigned long start, now;
	now = clockms();
	clock->time = now - clock->start;

	if (clock->waspaused == true) start = clock->pausedtime;
	else start = clock->start;

	clock->diff = now - start;
}	
