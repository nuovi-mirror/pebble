#include "appendstr.h"
#include "string.h"

char *appendstr
(const char *src, char *dst)
{ return strcat(dst, src); }
