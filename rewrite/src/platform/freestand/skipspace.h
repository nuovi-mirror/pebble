#ifndef PLATFORM_SKIPSPACE_H_
#define PLATFORM_SKIPSPACE_H_

#include "getstrlen.h"

/* function with old behavior for legacy compat */
char *skipspace
(char *p) 
{
	while (*p == ' ' || *p == '\t')
		p++;
	return (*p == '\0' ? NULL : p);
}

/* wrapper that is a bit more useful */
char *tskipspace
(const char *p)
{
	char *r = skipspace((char *)p);
	return r ? r : p + getstrlen(p);
}

#endif
