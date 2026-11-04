#include "ffi.h"
#include "limits.h"
#include "snooze.h"

void escape_std_misc_sleep 
(FFIvars *vars, unsigned long long seed, FFIArena *scratchAlloc, FFIArena *tempAlloc, FFIArena *persistAlloc, struct Limits *limits)
{
	FFIValue *msg = FFIresolvePointer(vars, "__Escape_std.misc.sleep_ARG0", tempAlloc, limits);
	FFIValue time = FFIconvertValueToWord(*msg, vars, persistAlloc, limits);

	unsigned long slept = snooze((unsigned long)time.as.word);

	FFIsetVariable(vars, "__Escape_std.misc.sleep_RET0", 
		(FFIValue){ .Type = type_word, .as.word = slept }, persistAlloc);
}
