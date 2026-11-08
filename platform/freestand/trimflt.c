#include "trimflt.h"

unsigned long trimflt
(char *buff, unsigned long len)
{
	unsigned long i = len;
	unsigned long j;

	if (len < 2 || buff[len - 1] != '0') return len; /* nothing to trim */

	while (i > 0 && buff[i - 1] == '0') i--; /* i: just past the last non-zero char */

	for (j = i; j > 0 && buff[j - 1] >= '0' && buff[j - 1] <= '9'; j--) ;
	if (j == 0 || buff[j - 1] != '.') return len;

	if (buff[i - 1] == '.') i++; /* keep one digit */

	buff[i] = '\0';
	return i;
}
