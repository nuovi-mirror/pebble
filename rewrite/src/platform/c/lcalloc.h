#ifndef PLATFORM_LCALLOC_H_
#define PLATFORM_LCALLOC_H_

#include <stdlib.h>

void *lcalloc(unsigned long nmemb, unsigned long size) {
	return calloc(nmemb, size);
}

#endif
