#pragma once

#include <string.h>

void *copymem
(void *dst, const void *src, size_t len)
{
	return memcpy(dst, src, len);
}
