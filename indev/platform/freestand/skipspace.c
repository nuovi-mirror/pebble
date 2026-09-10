#include "skipspace.h"
#include "getnstrlen.h"
#include "main.h"

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
(char *p)
{
	char *r = skipspace((char *)p);
	return r ? r : p + getnstrlen(p);
}
