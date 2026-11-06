#include "ffillmem.h"

void *setmem 
(void *b, int c, unsigned long len)
{ return ffillmem(c, b, len); }
