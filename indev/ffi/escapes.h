#ifndef FFI_ESCAPES_H_
#define FFI_ESCAPES_H_

#include "sequences.h"

typedef void (*EscapeEntry)(void);

struct EscapeSequence {
	const char *name;
	EscapeEntry entry;
};

static const struct EscapeSequence escapes[] = {
	{ "test.hello", escape_test_hello_ },
};

void callEscape(const char *name);

#endif
