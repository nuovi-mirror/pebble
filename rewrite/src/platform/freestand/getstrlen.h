#pragma once

unsigned long getstrlen(const char *s) {
	unsigned long n = 0;

	while (s[n] != '\0')
		n++;
	return n + 1;
}
