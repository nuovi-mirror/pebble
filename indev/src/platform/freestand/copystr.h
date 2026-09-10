#ifndef PLATFORM_COPYSTR_H_
#define PLATFORM_COPYSTR_H_

void copystr(const char *src, char *dst) {
	size_t i = 0;

	while(src[i] != '\0') {
		dst[i] = src[i];
		i++;
	}

	dst[i] = '\0';
}

#endif
