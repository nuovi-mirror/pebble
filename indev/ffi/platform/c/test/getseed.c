#include "ffi.h"
#include "limits.h"
#include "snprint.h"

void escape_test_getseed (FFIvars *vars, unsigned long long seed, FFIArena *scratchAlloc,
	FFIArena *tempAlloc, FFIArena *persistAlloc, struct Limits *limits)
{
	char *buff = alloc(tempAlloc, sizeof(seed) * 8);
	snprint(buff, sizeof(seed) * 8, "%llu", seed);
	FFIstdoutPrint(buff);
	FFIstdoutPrint("\n");
}
