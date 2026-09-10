#include <stdlib.h>
#include "lcalloc.h"

void *lcalloc(unsigned long nmemb, unsigned long size) {
	return calloc(nmemb, size);
}
