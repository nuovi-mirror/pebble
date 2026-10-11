#include "ffi.h"
#include "limits.h"
#include "cmpstr.h"
#include "copystr.h"

void escape_std_term_color_basic_back (FFIvars *vars, unsigned long long seed, FFIArena *scratchAlloc,
	FFIArena *tempAlloc, FFIArena *persistAlloc, struct Limits *limits)
{
	char *ptrbuff = FFIallocateMemory(tempAlloc, limits->instructions_varnamesize);
	char *msgbuff = FFIallocateMemory(tempAlloc, limits->instructions_varnamesize);

	FFIValue *ptr = FFIreadVariable(vars, "__Escape_std.term.color.basic.fore_ARG0");
	FFIconvertValueToString(ptrbuff, limits->instructions_varnamesize, *ptr);

	FFIValue *msg = FFIreadVariable(vars, ptrbuff);
	FFIconvertValueToString(msgbuff, limits->instructions_varnamesize, *msg);

	/* XXX very unoptimized, but optmizing this is just not worth it */

	char *code = alloc(tempAlloc, 8);

	if (cmpstr(msgbuff, "white") == 0) copystr("47", code);
	else if (cmpstr(msgbuff, "red") == 0) copystr("41", code);
	else if (cmpstr(msgbuff, "green") == 0) copystr("42", code);
	else if (cmpstr(msgbuff, "yellow") == 0) copystr("43", code);
	else if (cmpstr(msgbuff, "blue") == 0) copystr("44", code);
	else if (cmpstr(msgbuff, "magenta") == 0) copystr("45", code);
	else if (cmpstr(msgbuff, "cyan") == 0) copystr("46", code);
	else copystr("40", code); /* XXX CPebble-specific behavior - default to black */

	FFIstdoutPrint("\x1b[");
	FFIstdoutPrint(code);
	FFIstdoutPrint("m");
}
