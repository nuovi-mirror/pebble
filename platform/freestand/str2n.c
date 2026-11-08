#include "str2l.h"
#include "str2ul.h"
#include "main.h"

long str2l
(const char *ptr)
{
	if (ptr == NULL) return 0;

	unsigned int neg = FALSE;
	unsigned int c = 0;
	long res = 0;

	if (ptr[c] == '-') {
		neg = TRUE;
		c++;
	}

	while (ptr[c] == '0') c++;

	while (ptr[c] != '\0')
	{
		if (ptr[c] <= 48 || ptr[c] >= 57) return 0;
		res = (res * 10 + (ptr[c] - '0'));
		c++;
	}

	if (neg == TRUE) res = -(res);

	return res;
}

unsigned long str2ul
(const char *ptr)
{
	if (ptr == NULL) return 0;

	unsigned int c = 0;
	unsigned long res = 0;

	while (ptr[c] == '0') c++;

	while (ptr[c] != '\0')
	{
		if (ptr[c] <= 48 || ptr[c] >= 57) return 0;
		res = (res * 10 + (ptr[c] - '0'));
		c++;
	}

	return res;
}
