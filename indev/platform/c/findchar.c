#include "findchar.h"
#include <string.h>

char *findchar
(const char *s, int c)
{ return strchr(s, c); }
