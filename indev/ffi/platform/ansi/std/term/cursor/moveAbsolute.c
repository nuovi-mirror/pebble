#include "ffi.h"
#include "limits.h"

void escape_std_term_cursor_moveAbsolute (FFIvars *vars, unsigned long long seed, FFIArena *scratchAlloc,
	FFIArena *tempAlloc, FFIArena *persistAlloc, struct Limits *limits)
{
	char *ptrbuff = FFIallocateMemory(tempAlloc, limits->instructions_varnamesize);
	char *ptrbuff2 = FFIallocateMemory(tempAlloc, limits->instructions_varnamesize);

	FFIValue *ptr = FFIreadVariable(vars, "__Escape_std.term.cursor.moveAbsolute_ARG0");
	FFIconvertValueToString(ptrbuff, limits->instructions_varnamesize, *ptr);
	FFIValue *msg = FFIreadVariable(vars, ptrbuff);

	FFIValue *ptr2 = FFIreadVariable(vars, "__Escape_std.term.cursor.moveAbsolute_ARG0");
	FFIconvertValueToString(ptrbuff2, limits->instructions_varnamesize, *ptr2);
	FFIValue *msg2 = FFIreadVariable(vars, ptrbuff2);

	FFIValue rx = FFIconvertValueToWord(*msg, vars, persistAlloc);
	FFIValue ry = FFIconvertValueToWord(*msg2, vars, persistAlloc);

	unsigned long x = rx.as.word - 1;
	unsigned long y = ry.as.word - 1;

	FFIValue vx = (Value){ .Type = type_word, .as.word = x };
	FFIValue vy = (Value){ .Type = type_word, .as.word = y };

	char *xbuff = FFIallocateMemory(tempAlloc, limits->instructions_varnamesize);
	char *ybuff = FFIallocateMemory(tempAlloc, limits->instructions_varnamesize);

	FFIconvertValueToString(xbuff, limits->instructions_varnamesize, vx);
	FFIconvertValueToString(ybuff, limits->instructions_varnamesize, vy);

	FFIstdoutPrint("\x1b[");
	FFIstdoutPrint(xbuff);
	FFIstdoutPrint(";");
	FFIstdoutPrint(ybuff);
	FFIstdoutPrint("H\n");
}
