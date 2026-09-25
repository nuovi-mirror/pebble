#include "ffi.h"
#include "inputl.h"

#define MAX 512

void escape_std_io_input (FFIvars *vars, unsigned long long seed, FFIArena *scratchAlloc, 
	FFIArena *tempAlloc, FFIArena *persistAlloc) 
{
	char *chars = FFIallocateMemory(persistAlloc, MAX);
	inputl(chars, MAX);

	FFIallocateVariable(vars, "__Escape_std.io.input_RET0", &chars, persistAlloc);
}
