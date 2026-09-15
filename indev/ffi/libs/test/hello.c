#include "ffi.h"
#include "print.h"
#include "allocator.h"

void escape_test_hello(VMFFIvars *vars, Arena *scratchAlloc, Arena *tempAlloc, Arena *persistAlloc) 
{
	print("Hello from C!\n");
}
