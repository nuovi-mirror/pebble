#include "ffi.h"
#include "limits.h"
#include "cmpstr.h"
#include "copystr.h"

void escape_std_term_color_basic_fore (FFIvars *vars, unsigned long long seed, FFIArena *scratchAlloc,
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

	if (cmpstr(msgbuff, "black") == 0) copystr("30", code);
	else if (cmpstr(msgbuff, "red") == 0) copystr("31", code);
	else if (cmpstr(msgbuff, "green") == 0) copystr("32", code);
	else if (cmpstr(msgbuff, "yellow") == 0) copystr("33", code);
	else if (cmpstr(msgbuff, "blue") == 0) copystr("34", code);
	else if (cmpstr(msgbuff, "magenta") == 0) copystr("35", code);
	else if (cmpstr(msgbuff, "cyan") == 0) copystr("36", code);
	else copystr("37", code); /* XXX CPebble extension - default to white */

	FFIstdoutPrint("\x1b[");
	FFIstdoutPrint(code);
	FFIstdoutPrint("m");
}
