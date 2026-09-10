#include "copystr.h"

void copystr(const char *src, char *dst) {
	unsigned long i = 0;

	while(src[i] != '\0') {
		dst[i] = src[i];
		i++;
	}

	dst[i] = '\0';
}
