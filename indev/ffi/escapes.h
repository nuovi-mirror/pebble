#ifndef ffi_escapes_h_
#define ffi_escapes_h_

#include "ffi.h"
#include "sequences.h"
#include "limits.h"

typedef void (*EscapeEntry)(FFIvars *vars, unsigned long long seed, FFIArena *scratchAlloc,
	FFIArena *tempAlloc, FFIArena *persistAlloc, struct Limits *limits);

struct EscapeSequence {
	const char *name;
	EscapeEntry func;
};

static const struct EscapeSequence escapes[] = {
	/* debug libraries */
	{"test.hello", escape_test_hello},
	{"test.getseed", escape_test_getseed},

	/* standard libraries */
	{"std.io.print", escape_std_io_print},
	{"std.io.printLn", escape_std_io_printLn},

#ifndef noinput
	{"std.io.input", escape_std_io_input},
#endif

	{"std.misc.random", escape_std_misc_random},

#ifndef nosnooze
	{"std.misc.sleep", escape_std_misc_sleep},
#endif

#ifdef graphics
	/* graphics libraries */
	{"gs.2d.window.create", escape_gs_2d_window_create},
#endif

#ifdef terminal
	/* terminal libraries */
	{"std.term.color.basic.fore", escape_std_term_color_basic_fore},
	{"std.term.screen.clear", escape_std_term_screen_clear},
#endif
};

void callEscape (const char *name, unsigned long long seed, FFIvars *vars,
	FFIArena *scratchAlloc, FFIArena *tempAlloc, FFIArena *persistAlloc,
	struct Limits *limits);

#endif
