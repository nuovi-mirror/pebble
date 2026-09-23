#include "ffi.h"
#include "snprint.h"

void escape_test_getseed
(FFIvars *vars, unsigned long long seed, FFIArena *scratchAlloc, 
 FFIArena *tempAlloc, FFIArena *persistAlloc)
{
	char *buff = alloc(tempAlloc, sizeof(seed) * 8);
	snprint(buff, sizeof(seed) * 8, "%llu", seed);
	FFIstdoutPrint(buff);
	FFIstdoutPrint("\n");
}
