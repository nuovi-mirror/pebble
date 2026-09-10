#ifndef PLATFORM_LALLOC_H_
#define PLATFORM_LALLOC_H_

#include <stdlib.h>

void *lalloc(unsigned long size) {
	return malloc(size);
}

#endif
