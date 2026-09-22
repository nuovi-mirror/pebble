#include "readfile.h"
#include "copymem.h"
#include "limits.h"

/* there is no 'filesystem' here, so we just load it from 
 * this memory address */
#define FREESTAND_PROGRAM_ADDR ((const char *)0x00200000UL)

unsigned long getfilesize(const char *path)
{
	(void)path;

	const char *p = FREESTAND_PROGRAM_ADDR;
	unsigned long len = 0;

	while (len < limits_misc_maxfilebuffersize && p[len] != '\0')
		len++;

	return len;
}

char *readfile(const char *path, void *buf, unsigned long nbytes)
{
	(void)path;

	unsigned long len = getfilesize(0);

	/* mirror the hosted readfile's contract exactly: NULL on failure,
	 * NUL-terminated result, never write past nbytes */
	if (len == 0 || len >= nbytes)
		return 0;

	copymem(FREESTAND_PROGRAM_ADDR, buf, len);
	((char *)buf)[len] = '\0';

	return buf;
}
