#ifndef FFI_ESCAPES_H_
#define FFI_ESCAPES_H_

#include "ffi.h"
#include "sequences.h"

typedef void (*EscapeEntry)(FFIvars* vars, unsigned long long seed,
                            FFIArena* scratchAlloc, FFIArena* tempAlloc,
                            FFIArena* persistAlloc);

struct EscapeSequence {
  const char* name;
  EscapeEntry func;
};

static const struct EscapeSequence escapes[] = {
    /* debug libraries */
    {"test.hello", escape_test_hello},
    {"test.getseed", escape_test_getseed},

    /* standard libraries */
    {"std.io.print", escape_std_io_print},
    {"std.io.printLn", escape_std_io_printLn},

#ifdef graphics
    /* graphics libraries */
    {"gs.2d.window.create", escape_gs_2d_window_create},
#endif
};

void callEscape(const char* name, unsigned long long seed, FFIvars* vars,
                FFIArena* scratchAlloc, FFIArena* tempAlloc,
                FFIArena* persistAlloc);

#endif
