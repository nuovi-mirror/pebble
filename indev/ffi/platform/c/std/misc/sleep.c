#include "ffi.h"
#include "limits.h"
#include "snooze.h"

void escape_std_misc_sleep 
(FFIvars *vars, unsigned long long seed, FFIArena *scratchAlloc, FFIArena *tempAlloc, 
 FFIArena *persistAlloc, struct Limits *limits)
{
	char *ptrbuff = FFIallocateMemory(tempAlloc, limits->instructions_varnamesize);
	FFIValue *ptr = FFIreadVariable(vars, "__Escape_std.misc.sleep_ARG0");
	FFIconvertValueToString(ptrbuff, limits->instructions_varnamesize, *ptr);

	FFIValue *msg = FFIreadVariable(vars, ptrbuff);
	FFIValue time = FFIconvertValueToWord(*msg, vars, persistAlloc);

	unsigned long slept = snooze((unsigned long)time.as.word);

	FFIValue *ret = FFIallocateMemory(persistAlloc, sizeof(FFIValue) + (sizeof(type_word) * 8));
	*ret = (FFIValue){ .Type = type_word, .as.word = slept };

	FFIallocateVariable(vars, "__Escape_std.misc.sleep_RET0", ret, persistAlloc);
}
