/* debug libraries */
#include "libs/test/getseed.h"
#include "libs/test/hello.h"

/* standard libraries */
#include "libs/std/io/print.h"
#include "libs/std/io/printLn.h"

#ifndef noinput
#include "libs/std/io/input.h"
#endif /* ifdef noinput */

#include "libs/std/misc/random.h"

#ifndef nosnooze
#include "libs/std/misc/sleep.h"
#endif /* #ifndef nosnooze */

#ifndef notime
/* XXX not implimented 
#include "libs/std/misc/time.h" 
*/
#endif /* #ifndef notime */

#ifdef graphics
/* graphis libraries */
#include "libs/gs/2d/window/create.h"
#endif /* #ifdef graphics */

#ifdef terminal
/* terminal libraries */
#include "libs/std/term/color/basic/fore.h"
#include "libs/std/term/color/basic/back.h"
#include "libs/std/term/color/basic/reset.h"
#include "libs/std/term/cursor/moveAbsolute.h"
#include "libs/std/term/screen/clear.h"
#endif /* #ifdef terminal */

#ifdef proc
/* process libraries */
#include "libs/std/proc/spawn.h"
#include "libs/std/proc/wait.h"
#endif /* #ifdef proc */

#ifdef fs
/* filesystem libraries */
#include "libs/std/fs/file/write.h"
/* XXX not implimented
#include "libs/std/fs/file/read.h"
#include "libs/std/fs/file/writeBinary.h"
#include "libs/std/fs/dir/list.h"
#include "libs/std/fs/dir/get.h"
#include "libs/std/fs/dir/change.h"
*/
#endif /* #ifdef fs */
