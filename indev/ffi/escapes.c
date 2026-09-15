#include "escapes.h"
#include "cmpstr.h"
#include "allocator.h"
#include "sequences.h"

void callEscape
(const char *name, Arena *scratchAlloc, Arena *tempAlloc, Arena *persistAlloc)
{
	unsigned long i;

	for (i = 0; i < sizeof(escapes) / sizeof(escapes[0]); i++)
	{
		if (cmpstr(escapes[i].name, name) == 0) {
			escapes[i].func(scratchAlloc, tempAlloc, persistAlloc);
			return;
		}
	}
}
