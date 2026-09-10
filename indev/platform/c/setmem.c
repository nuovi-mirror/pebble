#include "setmem.h"
#include <string.h>

void *setmem
(void *b, int c, size_t len)
{
	return memset(b, c, len);
}
