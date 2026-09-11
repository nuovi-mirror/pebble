#include "readfile.h"
#include "main.h"
#include <stdio.h>

char *readfile(const char *path, void *buf, unsigned long nbytes) {
	FILE *f = fopen(path, "rb");
	if (!f) return NULL;
	unsigned long size_read = fread(buf, 1, nbytes - 1, f);
	fclose(f);
	if (size_read == 0) return NULL;
	((char *)buf)[size_read] = '\0';
	return buf;
}
