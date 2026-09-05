#ifndef PLATFORM_GETNSTRLEN_H_
#define PLATFORM_GETNSTRLEN_H_

unsigned long getnstrlen(const char *s) {
	unsigned long n = 0;

	while (s[n] != '\0')
		n++;
	return n;
}

#endif
