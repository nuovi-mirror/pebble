#include "escapes.h"
#include "cmpstr.h"

void callEscape
(const char *name)
{
	unsigned long i;

	for (i = 0; i < sizeof(escapes) / sizeof(escapes[0]); i++)
	{
		if (cmpstr(commands[i].name, name) == 0) {
			commands[i].func();
			return;
		}
	}
}
