#include <stdlib.h>
#include "lalloc.h"

void *lalloc(unsigned long size) {
	return malloc(size);
}
