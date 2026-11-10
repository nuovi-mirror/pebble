#include "ffi.h"
#include "limits.h"
#include "main.h"
#include "getstrlen.h"
#include "copymem.h"
#include <dirent.h>

void escape_std_fs_dir_list 
(FFIvars *vars, unsigned long long seed, FFIArena *scratchAlloc, FFIArena *tempAlloc, FFIArena *persistAlloc, struct Limits *limits)
{
	unsigned long maxsize = ((limits->misc_dirlist_maxsize) - 1);
	unsigned long currsize = 0;
	struct dirent *de;
	char *buff = FFIallocateMemory(tempAlloc, maxsize + 1);
	char *path = FFIallocateMemory(tempAlloc, limits->misc_strlen);
	char *arg0str = FFIallocateMemory(tempAlloc, limits->misc_strlen);

	FFIValue *arg0 = FFIresolvePointerUnsafe(vars, "__Escape_std.fs.dir.list_ARG0", tempAlloc, limits);
	if (arg0 == NULL) copymem(".", path, 2);
	else 
	{
		if (limits->misc_strlen < 2) goto err_noclean;
		FFIconvertValueToString(arg0str, limits->misc_strlen, *arg0, limits);
		copymem(arg0str, path, limits->misc_strlen);
	}

	DIR *dr = opendir(path);
	if (dr == NULL) goto err;


	while ((de = readdir(dr)) != NULL)
	{
		unsigned long dlen = getstrlen(de->d_name);
		if ((currsize + dlen + 1) >= maxsize) continue;
		copymem(de->d_name, (buff + currsize), dlen);
		currsize += dlen;
		buff[currsize++] = '\n';
		/* replace the terminating char with a newline */
	}

	buff[currsize] = '\0';
		
	unsigned long len = getstrlen(buff) + 1;
	char *ret = FFIallocateMemory(persistAlloc, len);
	if (len > maxsize) len = maxsize;
	copymem(buff, ret, len);

	FFIsetVariable(vars, "__Escape_std.fs.dir.list_RET0", (FFIValue){ .Type = type_word, .as.word = 0 }, persistAlloc);
	FFIsetVariable(vars, "__Escape_std.fs.dir.list_RET1", (FFIValue){ .Type = type_str, .as.str = ret }, persistAlloc);

	closedir(dr);
	return;

err:
	if (dr != NULL) closedir(dr);
err_noclean:
	FFIsetVariable(vars, "__Escape_std.fs.dir.list_RET0", (FFIValue){ .Type = type_word, .as.word = 1 }, persistAlloc);
	return;
}
