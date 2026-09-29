#include "ffi.h"
#include "limits.h"

void escape_test_getseed (FFIvars *vars, unsigned long long seed, FFIArena *scratchAlloc,
	FFIArena *tempAlloc, FFIArena *persistAlloc, struct Limits *limits);
