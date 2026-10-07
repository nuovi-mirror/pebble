/* debug libraries */
#include "libs/test/getseed.h"
#include "libs/test/hello.h"

/* standard libraries */
#include "libs/std/io/print.h"
#include "libs/std/io/printLn.h"
#include "libs/std/io/input.h"

#include "libs/std/misc/random.h"
#include "libs/std/misc/sleep.h"

#if graphics == sdl2
/* graphis libraries */
#include "libs/gs/2d/window/create.h"
#endif

#if terminal == asni
/* terminal libraries */
#include "libs/std/term/color/basic/fore.h"
#endif
