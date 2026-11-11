#ifndef h_clock_
#define h_clock_

typedef struct Clock
{
	unsigned long start;		/* start ms time */
	unsigned long end;		/* ended ms time */
	unsigned long curr;		/* current time */
	unsigned long diff;		/* start - end */
	unsigned long time;		/* time it has ran */
	unsigned int paused;		/* 0 = paused */
	unsigned int pausedtime;	/* ms time timer was paused */
	unsigned int waspaused;		/* was paused before */
} Clock;

void timer_init (Clock *clock);
void timer_start (Clock *clock);
void timer_pause (Clock *clock);
void timer_stop (Clock *clock);
void timer_unpause (Clock *clock);

#endif
