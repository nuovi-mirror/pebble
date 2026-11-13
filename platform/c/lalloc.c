#include "lalloc.h"
#include "main.h" /* error() */
#include <stdlib.h> /* malloc() */

#ifdef debug
unsigned long lallocTracker;
#endif

void *lalloc
(unsigned long size, unsigned int max)
{
	unsigned int i = 0;
	void *ptr = NULL;

	while (ptr == NULL) 
	{ /* XXX ran once at start, so start at max +1 */
		if (i >= (max + 1)) error(errfatal, errcodeallocatorerror, "lalloc() failed!");
		ptr = malloc(size);
		i++;
	}
#ifdef debug
	lallocTracker += size;
#endif

	/* XXX cannot return null; error() with errfatal does not return */
	return ptr;
}
