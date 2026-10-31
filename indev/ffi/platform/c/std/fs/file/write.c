#include "ffi.h"
#include "limits.h"
#include "files.h"
#include "getstrlen.h"
#include "unescape.h"

void escape_std_fs_file_write 
(FFIvars *vars, unsigned long long seed, FFIArena *scratchAlloc, FFIArena *tempAlloc, FFIArena *persistAlloc, struct Limits *limits)
{
	FFIValue *arg0 = FFIresolvePointer(vars, "__Escape_std.fs.file.write_ARG0", tempAlloc, limits);
	FFIValue *arg1 = FFIresolvePointer(vars, "__Escape_std.fs.file.write_ARG1", tempAlloc, limits);

	char *arg0str = FFIallocateMemory(tempAlloc, limits->misc_strlen);
	char *arg1str = FFIallocateMemory(tempAlloc, limits->misc_strlen);
	char *unescaped = FFIallocateMemory(tempAlloc, limits->misc_strlen);
	FFIconvertValueToString(arg0str, limits->misc_strlen, *arg0, limits);
	FFIconvertValueToString(arg1str, limits->misc_strlen, *arg1, limits);

	unsigned long datasize = unescape(arg1str, unescaped, limits->misc_strlen);
	long errcode;

	unsigned long wsize = writefile(arg0str, unescaped, datasize);
	if (wsize != datasize) errcode = 1;
	else errcode = 0;

	FFIsetVariable(vars, "__Escape_std.fs.file.write_RET0", (FFIValue){ .Type = type_word, .as.word = errcode }, persistAlloc);
}
