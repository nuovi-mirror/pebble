#include "copymem.h"

typedef unsigned long word; /* word size */

#define wordsize sizeof(word)
#define wordsizeminus wordsize - 1

void *copymem
(const void *src, void *dst, unsigned long len)
{
	const unsigned char *charsrc = src;
	unsigned char *chardst = dst;
	unsigned long count = 0;

	/* if aliagn differs, copy bytes */
	if (((unsigned long)charsrc ^ (unsigned long)chardst) & wordsizeminus)
		goto copybytes;

	/* align */
	count = (-((unsigned long)charsrc)) & wordsize;

	if (count > len)
		count = len;

	while (count--)
	{
		*chardst++ = *charsrc++;
		len--;
	}

	/* words */
	count = len / wordsize;

	while (count--)
	{
		*(word *)chardst = *(const word *)charsrc;
		chardst += wordsize;
		charsrc += wordsize;
	}

	/* tail */
copybytes:
	while (len--)
		*chardst++ = *charsrc++;

	return dst;
}
