#include <stdlib.h>
#include "lfree.h"

void lfree(void *ptr) {
	free(ptr);
}
