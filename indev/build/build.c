#include "build.h"

/* VM source code */
const char *vm_srcs[] = {
	"vm/vm.c",
	"vm/vmmain.c",
	"vm/getprogram.c",
	"vm/cli.c",
	
	"allocator/allocator.c",
/*	"allocator/mem.c", */
	
	"state/evaluator.c",
	"state/expressions.c",
	"state/functions.c",
	"state/hashmap.c",
	"state/instructionmapper.c",
	"state/instructions.c",
	"state/values.c",
	"state/variables.c",

	NULL
};

/* platform-specific source code */

/* freestanding platform source code */
const char *platform_freestand_srcs[] = {
	"platform/freestand/cmpstr.c",
	"platform/freestand/cmpstrn.c",
	"platform/freestand/copystr.c",
	"platform/freestand/findnewline.c",
	"platform/freestand/getnstrlen.c",
	"platform/freestand/getstrlen.c",
	"platform/freestand/skipspace.c",
	"platform/freestand/copymem.c",
	"platform/freestand/strsplit.c",
	"platform/freestand/inputl.c",
	"platform/freestand/ffillmem.c",

	NULL
};

/* c platform source code */
const char *platform_c_srcs[] = {
	"platform/c/entry.c",
	"platform/c/exitproc.c",
	"platform/c/setmem.c",
	"platform/c/print.c",
	"platform/c/readfile.c",
	"platform/c/str2ul.c",
	"platform/c/str2d.c",
	"platform/c/snprint.c",

	NULL
};

/* indrng randomization stuff */
const char *indrng_srcs[] = {
	"indrng/first.c",
	"indrng/shuffle.c",
	"indrng/mix64.c",

	NULL
};

/* posix platform source code */
const char *platform_posix_srcs[] = {
	"platform/posix/inputn.c",
	"platform/posix/snooze.c",

	NULL
};

/* ffi source code */
const char *ffi_base_srcs[] = {
	"ffi/ffi.c",
	"ffi/escapes.c",

	NULL
};

const char *ffi_ansi_srcs[] = {
	"ffi/platform/ansi/std/term/color/basic/fore.c",
	"ffi/platform/ansi/std/term/screen/clear.c",

	NULL
};


const char *ffi_c_srcs[] = {
	"ffi/platform/c/test/hello.c",
	"ffi/platform/c/test/getseed.c",

	"ffi/platform/c/std/io/print.c",
	"ffi/platform/c/std/io/printLn.c",
	"ffi/platform/c/std/io/input.c",

	"ffi/platform/c/std/misc/random.c",
	"ffi/platform/c/std/misc/sleep.c",

	NULL
};

const char *ffi_posix_srcs[] = {

	NULL
};


const char *ffi_sdl2_srcs[] = {
	"ffi/platform/sdl2/gs/2d/window/create.c",

	NULL
};
