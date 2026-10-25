#include "ffi.h"
#include "inputl.h"
#include "limits.h"

void escape_std_io_input 
(FFIvars *vars, unsigned long long seed, FFIArena *scratchAlloc, FFIArena *tempAlloc, FFIArena *persistAlloc, struct Limits *limits) 
{
	char *chars = FFIallocateMemory(persistAlloc, limits->misc_strlen);
	inputl(chars, limits->misc_strlen);

	FFIValue *val = alloc(persistAlloc, sizeof(FFIValue));
	*val = (FFIValue){ .Type = type_str, .as.str = chars };

	FFIallocateVariable(vars, "__Escape_std.io.input_RET0", val, persistAlloc);
}
