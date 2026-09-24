#include "readfile.h"

#include <stdio.h>

#include "main.h"

char *readfile (const char *path, void *buf, unsigned long nbytes)
{
	FILE *f = fopen(path, "rb");
	if (!f)
		return NULL;
	unsigned long size_read = fread(buf, 1, nbytes - 1, f);
	fclose(f);
	if (size_read == 0)
		return NULL;
	((char *)buf)[size_read] = '\0';
	return buf;
}

unsigned long getfilesize (const char *path)
{
	FILE *f = fopen(path, "rb");
	if (!f)
		return 0;

	if (fseek(f, 0, SEEK_END) != 0) {
		fclose(f);
		return 0;
	}

	long size = ftell(f);
	fclose(f);

	if (size < 0)
		return 0;
	return (unsigned long)size;
}
