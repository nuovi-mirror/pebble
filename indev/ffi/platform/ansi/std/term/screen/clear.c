#include "ffi.h"
#include "limits.h"

void escape_std_term_screen_clear (FFIvars *vars, unsigned long long seed, FFIArena *scratchAlloc,
	FFIArena *tempAlloc, FFIArena *persistAlloc, struct Limits *limits)
{
	FFIstdoutPrint("\x1b[2J");
}
