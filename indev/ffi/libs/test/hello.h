#ifndef ESCAPE_TEST_HELLO_
#define ESCAPE_TEST_HELLO_

#include "ffi.h"

void escape_test_hello
(FFIvars *vars, unsigned long long seed, FFIArena *scratchAlloc, 
 FFIArena *tempAlloc, FFIArena *persistAlloc);

#endif
