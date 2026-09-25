#include "inputl.h"
#include "inputn.h"

unsigned long inputl (char *buff, unsigned long size) {
	char c;
	unsigned long i = 0;

	while (i < size) {
		if (inputn(&c, 1) != 1)
			break;

		if (c == '\n')
			break;

		buff[i] = c;
		i++;
	}

	return i;
}
