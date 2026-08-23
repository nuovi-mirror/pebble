#pragma once

size_t getstrlen(char *s) {
	size_t n = 0;

	while (s[n] != '\0')
		n++;
	return n + 1;
}
