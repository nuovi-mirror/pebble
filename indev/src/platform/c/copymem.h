#ifndef PLATFORM_COPYMEM_H_
#define PLATFORM_COPYMEM_H_

#include <string.h>

void *copymem
(const void *src, void *dst, size_t len)
{
	return memcpy(dst, src, len);
}

#endif
