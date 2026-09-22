#include "escapes.h"
#include "cmpstr.h"
#include "sequences.h"

void callEscape
(const char *name, unsigned long long seed, FFIvars *vars, FFIArena *scratchAlloc, 
 FFIArena *tempAlloc, FFIArena *persistAlloc)
{
	unsigned long i;

	for (i = 0; i < sizeof(escapes) / sizeof(escapes[0]); i++)
	{
		if (cmpstr(escapes[i].name, name) == 0) {
			escapes[i].func(vars, seed, scratchAlloc, 
					tempAlloc, persistAlloc);
			return;
		}
	}
}
