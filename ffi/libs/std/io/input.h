#include "ffi.h"
#include "limits.h"

void escape_std_io_input (FFIvars *vars, unsigned long long seed, FFIArena *scratchAlloc, 
	FFIArena *tempAlloc, FFIArena *persistAlloc, struct Limits *limits);
