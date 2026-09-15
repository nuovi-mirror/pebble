#ifndef FFI_ESCAPES_H_
#define FFI_ESCAPES_H_

#include "sequences.h"
#include "allocator.h"

typedef void (*EscapeEntry)
	(Arena *scratchAlloc, Arena *tempAlloc, Arena *persistAlloc);

struct EscapeSequence {
	const char *name;
	EscapeEntry func;
};

static const struct EscapeSequence escapes[] = {
	{ "test.hello", escape_test_hello },
};

void callEscape
(const char *name, Arena *scratchAlloc, Arena *tempAlloc, Arena *persistAlloc);

#endif
