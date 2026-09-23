#include "escapes.h"
#include "cmpstr.h"
#include "sequences.h"
#include "getstrlen.h"

static unsigned long long mix64
(unsigned long long x)
{
    x ^= x >> 30;
    x *= 0xBF58476D1CE4E5B9ULL;
    x ^= x >> 27;
    x *= 0x94D049BB133111EBULL;
    x ^= x >> 31;

    return x;
}

void callEscape
(const char *name, unsigned long long seed, FFIvars *vars, FFIArena *scratchAlloc, 
 FFIArena *tempAlloc, FFIArena *persistAlloc)
{
	unsigned long i;

	for (i = 0; i < sizeof(escapes) / sizeof(escapes[0]); i++)
	{
		if (cmpstr(escapes[i].name, name) == 0) {
			int len = getstrlen(name);
			unsigned long n = len < sizeof(unsigned long long) ? len : sizeof(unsigned long long);
			const unsigned char *lname = (const unsigned char *)name + len - n;
			unsigned long nseed = 0;

			for (unsigned long i = 0; i < n; i++)
				nseed = (nseed << sizeof(unsigned long) | lname[i]);

			unsigned long long ffiseed = mix64(seed ^ nseed);
			escapes[i].func(vars, ffiseed, scratchAlloc, 
					tempAlloc, persistAlloc);
			return;
		}
	}
}

