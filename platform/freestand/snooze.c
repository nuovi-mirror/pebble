#ifdef nosnooze
#include "snooze.h"
#include "main.h"

unsigned long snooze
(unsigned long ms)
{
	error(errwarning, errcodeundefined, "-Dnosnooze was included; cannot sleep!");
	return ms; /* fake it til' we make it */
}

#endif
