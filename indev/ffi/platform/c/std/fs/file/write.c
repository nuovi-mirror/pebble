#include "ffi.h"
#include "limits.h"
#include "files.h"
#include "getstrlen.h"

void escape_std_fs_file_write 
(FFIvars *vars, unsigned long long seed, FFIArena *scratchAlloc, FFIArena *tempAlloc, FFIArena *persistAlloc, struct Limits *limits)
{
	FFIValue *arg0 = FFIresolvePointer(vars, "__Escape_std.fs.file.write_ARG0", tempAlloc, limits);
	FFIValue *arg1 = FFIresolvePointer(vars, "__Escape_std.fs.file.write_ARG1", tempAlloc, limits);

	char *arg0str = FFIallocateMemory(tempAlloc, limits->misc_strlen);

	unsigned long wsize = writefile(arg0str, arg1->as.str, limits->file_maxsize);
	unsigned long errcode = 0;

	if (wsize == 0) errcode = 1;

	FFIsetVariable(vars, "__Escape_std.fs.file.write_RET0", (FFIValue){ .Type = type_word, .as.word = errcode }, persistAlloc);
}
