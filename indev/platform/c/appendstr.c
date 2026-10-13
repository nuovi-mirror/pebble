#include "appendstr.h"
#include "string.h"

char *appendstr
(src, dst)
const char *src;
char *dst;
{ return strcat(dst, src); }
