#include "ffi.h"

void escape_test_hello(FFIvars* vars, unsigned long long seed,
                       FFIArena* scratchAlloc, FFIArena* tempAlloc,
                       FFIArena* persistAlloc) {
  FFIstdoutPrint("Hello from C!\n");
}
