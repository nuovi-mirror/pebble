#pragma once

#include <unistd.h>
#include <fcntl.h>

char *readfile(const char *path, void *buf, size_t nbytes) {
	int d = open(path, O_RDONLY);

	if (d == -1)
		return NULL;

	int size_read = read(d, buf, nbytes - 1);
	close(d);

	if (size_read == -1 || size_read == 0)
		return NULL;

	((char *)buf)[size_read] = '\0';	/* append null-terminator */

	return buf;
}
