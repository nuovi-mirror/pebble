#pragma once

#include <stdlib.h>

void *lcalloc(size_t nmemb, size_t size) {
	return calloc(nmemb, size);
}
