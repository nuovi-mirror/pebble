#include "ffi.h"
#include "limits.h"

void escape_std_io_print
(FFIvars *vars, FFIArena *scratchAlloc, 
 FFIArena *tempAlloc, FFIArena *persistAlloc) 
{
	FFIValue *var = FFIreadVariable(vars, "__Escape_std.io.print_ARG0");
	char *buff = FFIallocateMemory(tempAlloc, limits_instructions_varnamesize);
	FFIconvertValueToString(buff, limits_instructions_varnamesize, *var);
	FFIstdoutPrint(buff);
	FFIstdoutPrint("\n");
}
