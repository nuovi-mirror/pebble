#include "trimflt.h"

unsigned long trimflt
(char *buff, unsigned long len)
{
	if (len <= 0) return len;
	if (buff[len - 1] != 0) return len;

	unsigned long i = len;
	unsigned long j;

	while (i > 0 && buff[i - 1] == '0') i--;

	for (j = i; j > 0 && buff[j - 1] >= '0' && buff[j - 1 <= '9']; j--) ;
	if (j == 0 || buff[j - 1] != '.') return len;

	if (buff[i - 1] == '.') i++;

	buff[i] = '\0';
	return i;
}
