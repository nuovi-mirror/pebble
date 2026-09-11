#include "copymem.h"

typedef unsigned long word; /* word size */

#define wordsize sizeof(word)
#define wordmask (wordsize - 1)

void *copymem
(const void *src, void *dst, unsigned long len)
{
	const unsigned char *charsrc = src;
	unsigned char *chardst = dst;
	unsigned long count = 0;

	/* why did you give me this? */
	if (len == 0 || src == dst)
		return dst;

	/* if aliagn differs, copy bytes */
	if ((((unsigned long)charsrc ^ (unsigned long)chardst) & wordmask) != 0 ) 
		goto copybytes;

	/* align */
	count = (wordsize - ((unsigned long)charsrc & wordmask)) & wordmask;

	if (count > len)
		count = len;

	while (count != 0)
	{
		*chardst++ = *charsrc++;
		--count;
		--len;
	}

	/* words */
	count = len / wordsize;

	while (count != 0)
	{
		*(word *)chardst = *(const word *)charsrc;
		chardst += wordsize;
		charsrc += wordsize;
		--count;
		len -= wordsize;
	}

	/* copy remaining bytes */
copybytes:
	while (len != 0) 
	{
		*chardst++ = *charsrc++;
		--len;
	}

	return dst;
}
