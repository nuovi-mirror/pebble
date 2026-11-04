#include "ffi.h"
#include "copymem.h"
#include "inputl.h"
#include "limits.h"

void escape_std_io_input 
(FFIvars *vars, unsigned long long seed, FFIArena *scratchAlloc, FFIArena *tempAlloc, FFIArena *persistAlloc, struct Limits *limits) 
{
	/* inputl() does not terminate what it reads, and fills the whole buffer if it can, so 
	 * leave room for the terminator and add it ourselves. (temp memory is not zeroed) */
	char *line = FFIallocateMemory(tempAlloc, limits->misc_strlen);
	unsigned long len = inputl(line, limits->misc_strlen - 1);
	line[len] = '\0';

	/* the string has to outlive this instruction, so it goes in the persistent arena, 
	 * but keep only what was typed and not the whole buffer. it is a new string every 
	 * call (not reused) so earlier copies of it never change */
	char *chars = FFIallocateMemory(persistAlloc, len + 1);
	copymem(line, chars, len);
	chars[len] = '\0';

	FFIsetVariable(vars, "__Escape_std.io.input_RET0", 
		(FFIValue){ .Type = type_str, .as.str = chars }, persistAlloc);
}
