#ifndef PLATFORM_LRSIZE_H_
#define PLATFORM_LRSIZE_H_

#include <stdlib.h>

void *lrsize(void *ptr, unsigned long size) {
	return realloc(ptr, size);
}

#endif
