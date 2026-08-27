#pragma once

#include <stdlib.h>

void *lrsize(void *ptr, unsigned long size) {
	return realloc(ptr, size);
}
