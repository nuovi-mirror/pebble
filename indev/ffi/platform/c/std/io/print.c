#include "ffi.h"
#include "limits.h"

void escape_std_io_print (FFIvars *vars, unsigned long long seed, FFIArena *scratchAlloc,
	FFIArena *tempAlloc, FFIArena *persistAlloc, struct Limits *limits)
{
	char *ptrbuff = FFIallocateMemory(tempAlloc, limits->instructions_varnamesize);
	char *msgbuff = FFIallocateMemory(tempAlloc, limits->instructions_varnamesize);

	FFIValue *ptr = FFIreadVariable(vars, "__Escape_std.io.print_ARG0");
	FFIconvertValueToString(ptrbuff, limits->instructions_varnamesize, *ptr);

	FFIValue *msg = FFIreadVariable(vars, ptrbuff);
	FFIconvertValueToString(msgbuff, limits->instructions_varnamesize, *msg);

	FFIstdoutPrint(msgbuff);
	FFIstdoutPrint("\n");
}
