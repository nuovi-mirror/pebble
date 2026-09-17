#ifndef FFI_ESCAPES_H_
#define FFI_ESCAPES_H_

#include "sequences.h"
#include "ffi.h"
#include "allocator.h"

typedef void (*EscapeEntry)
	(FFIvars *vars, FFIArena *scratchAlloc, 
	 FFIArena *tempAlloc, FFIArena *persistAlloc);

struct EscapeSequence {
	const char *name;
	EscapeEntry func;
};

static const struct EscapeSequence escapes[] = {
	{ "test.hello", escape_test_hello },
	{ "std.io.print", escape_std_io_print },
};

void callEscape
(const char *name, FFIvars *vars, FFIArena *scratchAlloc, 
 FFIArena *tempAlloc, FFIArena *persistAlloc);

#endif
