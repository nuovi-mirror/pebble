/* debug libraries */
#include "libs/test/getseed.h"
#include "libs/test/hello.h"

/* standard libraries */
#include "libs/std/io/print.h"
#include "libs/std/io/printLn.h"

#ifndef noinput
#include "libs/std/io/input.h"
#endif

#include "libs/std/misc/random.h"

#ifndef nosnooze
#include "libs/std/misc/sleep.h"
#endif

#ifdef graphics
/* graphis libraries */
#include "libs/gs/2d/window/create.h"
#endif

#ifdef terminal
/* terminal libraries */
#include "libs/std/term/color/basic/fore.h"
#include "libs/std/term/screen/clear.h"
#endif
