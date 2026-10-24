#include "ffi.h"
#include "limits.h"

void escape_std_io_print (FFIvars *vars, unsigned long long seed, FFIArena *scratchAlloc, FFIArena *tempAlloc, FFIArena *persistAlloc, struct Limits *limits)
{
	FFIValue *arg0 = FFIresolvePointer(vars, "__Escape_std.io.print_ARG0", tempAlloc, limits);
	FFIstdoutPrintValue(arg0, tempAlloc, limits);
	FFIstdoutPrint("\n");
}
