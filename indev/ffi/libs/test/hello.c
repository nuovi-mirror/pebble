#include "ffi.h"

void escape_test_hello
(FFIvars *vars, FFIArena *scratchAlloc, 
 FFIArena *tempAlloc, FFIArena *persistAlloc) 
{
	FFIstdoutPrint("Hello from C!\n");
}
