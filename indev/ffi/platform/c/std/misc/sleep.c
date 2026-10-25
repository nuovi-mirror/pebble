#include "ffi.h"
#include "limits.h"
#include "snooze.h"

void escape_std_misc_sleep 
(FFIvars *vars, unsigned long long seed, FFIArena *scratchAlloc, FFIArena *tempAlloc, FFIArena *persistAlloc, struct Limits *limits)
{
	FFIValue *msg = FFIresolvePointer(vars, "__Escape_std.misc.sleep_ARG0", tempAlloc, limits);
	FFIValue time = FFIconvertValueToWord(*msg, vars, persistAlloc, limits);

	unsigned long slept = snooze((unsigned long)time.as.word);

	FFIValue *ret = FFIallocateMemory(persistAlloc, sizeof(FFIValue) + (sizeof(type_word) * 8));
	*ret = (FFIValue){ .Type = type_word, .as.word = slept };

	FFIallocateVariable(vars, "__Escape_std.misc.sleep_RET0", ret, persistAlloc);
}
