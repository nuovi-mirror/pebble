#include <stdlib.h>
#include "exitproc.h"

_Noreturn void exitproc(int status) {
	exit(status);
}
