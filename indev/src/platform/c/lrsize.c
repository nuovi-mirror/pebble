#include <stdlib.h>
#include "lrsize.h"

void *lrsize(void *ptr, unsigned long size) {
	return realloc(ptr, size);
}
