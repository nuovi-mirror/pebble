#include <stdio.h>
#include "files.h"
#include "main.h"

char *readfile 
(const char *path, void *buf, unsigned long nbytes) 
{
	FILE *f = fopen(path, "rb");
	if (!f) return NULL;
	
	unsigned long size_read = fread(buf, 1, nbytes - 1, f);
	fclose(f);
	
	if (size_read == 0) return NULL;
	
	((char *)buf)[size_read] = '\0';
	return buf;
}

unsigned long getfilesize 
(const char *path) 
{
	FILE *f = fopen(path, "rb");
	if (!f) return 0;
	
	if (fseek(f, 0, SEEK_END) != 0) 
	{
		fclose(f);
		return 0;
	}

	long size = ftell(f);
	fclose(f);

	if (size < 0) return 0;
	return (unsigned long)size;
}

unsigned long writefile
(const char *path, const char *data, unsigned long datasize)
{
	unsigned long wsize;

	if (datasize == 0) return 0;
	if (data == NULL) return 0;

	FILE *f = fopen(path, "wb");
	if (!f) return 0;

	/*
	unsigned long rsize = getfilesize(path);
	if (getfilesize(path) > datasize) wsize = rsize;
	else wsize = datasize;
	*/

	wsize = datasize;

	unsigned long wtnsize = fwrite(data, 1, wsize, f);

	if (fclose(f) != 0) return 0;
	return wtnsize;
}
