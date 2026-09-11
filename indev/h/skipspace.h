#ifndef PLATFORM_SKIPSPACE_H_
#define PLATFORM_SKIPSPACE_H_

#include "getnstrlen.h"

/* function with old behavior for legacy compat */
char *skipspace
(char *p);

/* wrapper that is a bit more useful */
char *tskipspace
(char *p);

#endif
