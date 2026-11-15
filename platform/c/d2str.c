#include "str2n.h"
#include "snprint.h"

int d2str
(char *buff, unsigned long buffsize, double n)
{ return snprint(buff, buffsize, "%d", n); }
