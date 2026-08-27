#pragma once

#include <stdlib.h>

void *lcalloc(unsigned long nmemb, unsigned long size) {
	return calloc(nmemb, size);
}
