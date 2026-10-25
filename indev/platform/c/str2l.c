#include "str2l.h"
#include <stdlib.h>

long str2l 
(const char *nptr, char **endptr, int base)
{ return strtol(nptr, endptr, base); }
