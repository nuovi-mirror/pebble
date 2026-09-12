#include "str2ul.h"
#include <stdlib.h>
	
unsigned long str2ul
(const char *nptr, char **endptr, int base)
{ return strtoul(nptr, endptr, base); }
