#include "copymem.h"

void *copymem
(const void *src, void *dst, unsigned long len)
{
	const unsigned long *wsrc = (const unsigned long *)src;
	unsigned long *wdst = (unsigned long *)dst;
	unsigned long words = len / sizeof(unsigned long);

	while (words--)
		*wdst++ = *wsrc;

	/* remaining bytes */
	const unsigned char *bsrc = (const unsigned char *)wsrc;
	unsigned char *bdst = (unsigned char *)wdst;
	unsigned long bytes = len % sizeof(unsigned long);

	while (bytes--)
		*bdst++ = *bsrc++;

	return dst;
}
