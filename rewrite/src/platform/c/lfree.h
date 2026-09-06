#ifndef PLATFORM_LFREE_H_
#define PLATFORM_LFREE_H_

#include <stdlib.h>

void lfree(void *ptr) {
	free(ptr);
}

#endif
