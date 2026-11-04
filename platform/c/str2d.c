#include "str2d.h"
#include <stdlib.h>

double str2d
(const char *nptr, char **endptr)
{ return strtod(nptr, endptr); }
