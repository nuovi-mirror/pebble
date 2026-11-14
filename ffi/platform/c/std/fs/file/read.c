#include "ffi.h"
#include "limits.h"
#include "files.h"
#include "getstrlen.h"
#include "escape.h"

void escape_std_fs_file_read 
(FFIvars *vars, unsigned long long seed, FFIArena *scratchAlloc, FFIArena *tempAlloc, FFIArena *persistAlloc, struct Limits *limits)
{
	char *arg0str = FFIallocateMemory(tempAlloc, limits->misc_strlen);
	FFIValue *arg0 = FFIresolvePointer(vars, "__Escape_std.fs.file.read_ARG0", tempAlloc, limits);
	FFIconvertValueToString(arg0str, limits->misc_strlen, *arg0, limits);

	char *path = arg0str;

	unsigned long fsize = getfilesize(path) + 1;
	if (fsize == 0) goto err;

	char *fdata = FFIallocateMemory(tempAlloc, fsize);
	if (readfile(path, fdata, fsize) == 0) goto err;

	FFIsetVariable(vars, "__Escape_std.fs.file.read_RET0", (FFIValue){ .Type = type_word, .as.str = 0 }, persistAlloc);
	FFIsetVariable(vars, "__Escape_std.fs.file.read_RET1", (FFIValue){ .Type = type_str, .as.str = fdata }, persistAlloc);
	return;

err:
	FFIsetVariable(vars, "__Escape_std.fs.file.read_RET0", (FFIValue){ .Type = type_word, .as.word = 1 }, persistAlloc);
	FFIsetVariable(vars, "__Escape_std.fs.file.read_RET1", (FFIValue){ .Type = type_word, .as.word = 0 }, persistAlloc);
	return;
}
