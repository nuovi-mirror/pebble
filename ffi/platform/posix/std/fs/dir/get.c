#include "ffi.h"
#include "limits.h"
#include "main.h"
#include "getstrlen.h"
#include "copymem.h"
#include <unistd.h>

void escape_std_fs_dir_get 
(FFIvars *vars, unsigned long long seed, FFIArena *scratchAlloc, FFIArena *tempAlloc, FFIArena *persistAlloc, struct Limits *limits)
{
	char *buff = FFIallocateMemory(tempAlloc, limits->misc_strlen);
	if (getcwd(buff, limits->misc_strlen) == NULL) FFIerror(errfatal, errcodeplatformspecific, "Cannot getcwd(3)!");
	unsigned long len = getstrlen(buff);
	char *ret = FFIallocateMemory(persistAlloc, len);
	if (len > limits->misc_strlen) len = limits->misc_strlen;
	copymem(buff, ret, len);
	FFIsetVariable(vars, "__Escape_std.fs.dir.get_RET0", (FFIValue){ .Type = type_str, .as.str = ret }, persistAlloc);
}
