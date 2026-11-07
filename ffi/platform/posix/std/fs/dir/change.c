#include "ffi.h"
#include "limits.h"
#include "main.h"
#include "getstrlen.h"
#include "copymem.h"
#include <unistd.h>

void escape_std_fs_dir_change 
(FFIvars *vars, unsigned long long seed, FFIArena *scratchAlloc, FFIArena *tempAlloc, FFIArena *persistAlloc, struct Limits *limits)
{
	int code;
	char *arg0str;
	unsigned long ret;

	FFIValue *arg0 = FFIresolvePointer (vars, "__Escape_std.fs.dir.change_ARG0", tempAlloc, limits);
	arg0str = FFIallocateMemory(tempAlloc, limits->misc_strlen);
	FFIconvertValueToString(arg0str, limits->misc_strlen, *arg0, limits);
	code = chdir(arg0str);
	if (code == 0) ret = 0;
	else ret = 1;

	FFIsetVariable(vars, "__Escape_std.fs.dir.change_RET0", (FFIValue){ .Type = type_word, .as.word = ret }, persistAlloc);
}
