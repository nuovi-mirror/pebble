#include "ffi.h"
#include "limits.h"
#include "files.h"
#include "getstrlen.h"

unsigned long unescape
(const char *src, char *dest, unsigned long destsize)
{
	unsigned long j = 0, i = 0;

	if (src == NULL || dest == NULL || destsize == 0) return 0;

	while (src[i] != '\0' && j < destsize - 1)
	{
		if (src[i] == '\\' && src[i + 1] != '\0')
		{
			i++;

			switch(src[i])
			{
				case 'n': dest[j++] = '\n'; break;
				case 't': dest[j++] = '\t'; break;
				case 'r': dest[j++] = '\r'; break;
				case '\\': dest[j++] = '\\'; break;
				default:
					dest[j++] = '\\';
					if (j < destsize - 1) dest[j++] = src[i];
				break;

			}

			i++;
		}
		else dest[j++] = src[i++];
	}

	dest[j] = '\0';
	return j;
}

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
