#include "escape.h"
#include "main.h"

unsigned long escape
(const char *src, char *dest, unsigned long destsize)
{
	unsigned long j = 0, i = 0;

	if (src == NULL || dest == NULL || destsize == 0) return 0;

	while (src[i] != '\0' && j < destsize - 1)
	{
		if (src[i] == '\\' && src[i + 1] != '\0')
		{
			i++;

			switch(src[i])
			{
				case 'n': dest[j++] = '\n'; break;
				case 't': dest[j++] = '\t'; break;
				case 'r': dest[j++] = '\r'; break;
				case '\\': dest[j++] = '\\'; break;
				default:
					dest[j++] = '\\';
					if (j < destsize - 1) dest[j++] = src[i];
				break;

			}

			i++;
		}
		else dest[j++] = src[i++];
	}

	dest[j] = '\0';
	return j;
}
