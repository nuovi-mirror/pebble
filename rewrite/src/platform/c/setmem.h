#ifndef PLATFORM_SETMEM_
#define PLATFORM_SETMEM_

#include <strings.h>

void *setmem
(void *b, int c, size_t len)
{
	return memset(b, c, len);
}

#endif
