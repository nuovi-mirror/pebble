#include "copymem.h"
#include <string.h>

void *copymem
(const void *src, void *dst, unsigned long len)
{
	return memcpy(dst, src, len);
}
