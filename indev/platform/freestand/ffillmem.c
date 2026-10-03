#include "ffillmem.h"

typedef unsigned long word; /* word size */

#define wordsize sizeof(word)
#define wordmask (wordsize - 1)

void *ffillmem 
(unsigned char src, volatile void *dst, unsigned long len)
{
	volatile unsigned char *chardst = dst;
	unsigned long count = 0;
	word fill = 0;

	/* why did you give me this? */
	if (len == 0) return (void *)dst;

	for (; count < wordsize; count++)
		fill = (fill << 8) | src;
		

	/* align */
	count = (wordsize - ((unsigned long)chardst & wordmask)) & wordmask;

	if (count > len) count = len;

	while (count != 0) {
		*chardst++ = src;
		--count;
		--len;
	}

	/* words */
	count = len / wordsize;

	while (count != 0) {
		*(volatile word *)chardst = (const word )fill;
		chardst += wordsize;
		--count;
		len -= wordsize;
	}

	/* copy remaining bytes */
	while (len != 0) {
		*chardst++ = src;
		--len;
	}

	return (void *)dst;
}
