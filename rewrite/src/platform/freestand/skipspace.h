#pragma once

char *skipspace(char *p) {
	while (*p == ' ' || *p == '\t')
		p++;
	return (*p == '\0' ? NULL : p);
}
