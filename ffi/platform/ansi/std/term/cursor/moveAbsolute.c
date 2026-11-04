#include "ffi.h"
#include "limits.h"

void escape_std_term_cursor_moveAbsolute 
(FFIvars *vars, unsigned long long seed, FFIArena *scratchAlloc, FFIArena *tempAlloc, FFIArena *persistAlloc, struct Limits *limits)
{
	FFIValue *msg = FFIresolvePointer(vars, "__Escape_std.term.cursor.moveAbsolute_ARG0", tempAlloc, limits);
	FFIValue *msg2 = FFIresolvePointer(vars, "__Escape_std.term.cursor.moveAbsolute_ARG1", tempAlloc, limits);

	FFIValue rx = FFIconvertValueToWord(*msg, vars, persistAlloc, limits);
	FFIValue ry = FFIconvertValueToWord(*msg2, vars, persistAlloc, limits);

	unsigned long x = rx.as.word - 1;
	unsigned long y = ry.as.word - 1;

	FFIValue vx = (Value){ .Type = type_word, .as.word = x };
	FFIValue vy = (Value){ .Type = type_word, .as.word = y };

	char *xbuff = FFIallocateMemory(tempAlloc, limits->instructions_varnamesize);
	char *ybuff = FFIallocateMemory(tempAlloc, limits->instructions_varnamesize);

	FFIconvertValueToString(xbuff, limits->instructions_varnamesize, vx, limits);
	FFIconvertValueToString(ybuff, limits->instructions_varnamesize, vy, limits);

	FFIstdoutPrint("\x1b[");
	FFIstdoutPrint(xbuff);
	FFIstdoutPrint(";");
	FFIstdoutPrint(ybuff);
	FFIstdoutPrint("H\n");
}
