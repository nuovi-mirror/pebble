#include "ffi.h"
#include "limits.h"

static unsigned long mix64 
(unsigned long long x)
{
	x ^= x >> 30;
	x *= 0xBF58476D1CE4E5B9ULL;
	x ^= x >> 27;
	x *= 0x94D049BB133111EBULL;
	x ^= x >> 31;

	return x;
}
void escape_std_misc_random 
(FFIvars *vars, unsigned long long seed, FFIArena *scratchAlloc, FFIArena *tempAlloc, 
 FFIArena *persistAlloc, struct Limits *limits)
{
	unsigned long long retseed;

	FFIValue *ptr = FFIreadVariableUnsafe(vars, "__Escape_std.misc.random_seed");
	
	if (ptr != NULL) {
		FFIValue v = FFIconvertValueToWord(*ptr, vars, persistAlloc);
		retseed = (unsigned long long)v.as.word;
	} else retseed = seed;

	retseed = mix64(retseed ^ 0xB81CAF4C3);

	FFIValue *ret = alloc(persistAlloc, sizeof(FFIValue));
	*ret = (FFIValue){ .Type = type_word, .as.word = retseed };

	FFIValue *rseed = alloc(persistAlloc, sizeof(FFIValue));
	*rseed = (FFIValue){ .Type = type_word, .as.word = retseed };

	FFIallocateVariable(vars, "__Escape_std.misc.random_RET0", ret, persistAlloc);
	FFIallocateVariable(vars, "__Escape_std.misc.random_seed", rseed, persistAlloc);
}

