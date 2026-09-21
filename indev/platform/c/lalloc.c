#include "lalloc.h"
#include <stdlib.h>

void *lalloc
(unsigned long size) 
{
	return malloc(size);
}
