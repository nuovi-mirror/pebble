#pragma once

#include <stdlib.h>

void lfree(void *ptr) {
	return free(ptr);
}
