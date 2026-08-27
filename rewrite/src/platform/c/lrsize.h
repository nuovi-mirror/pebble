#pragma once

#include <stdlib.h>

void *lrsize(void *ptr, size_t size) {
	return realloc(ptr, size);
}
