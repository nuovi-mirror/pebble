#include "escape.h"
#include "main.h"

unsigned long escape
(const char *src, char *dest, unsigned long destsize)
{
	unsigned long j = 0, i = 0;

	if (src == NULL || dest == NULL || destsize == 0) return 0;

	while (src[i] != '\0' && j < destsize - 1)
	{
		switch(src[i])
		{
			case '\n': 
				dest[j++] = '\\';
				dest[j++] = 'n';
				break;

			case '\t': 
				dest[j++] = '\\';
				dest[j++] = 't';
				break;


			case '\r': 

				dest[j++] = '\\';
				dest[j++] = 'r';
				break;

			default:
				dest[j++] = src[i];
				break;
		}

		i++;
	}

	dest[j] = '\0';
	return j;
}
