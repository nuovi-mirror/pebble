#pragma once

char *findnewline(char **cursor) {
	if (**cursor == '\0')
		return NULL;				/* nothing left */

	char *start = *cursor;
	char *p = *cursor;

	while (*p != '\n' && *p != '\0')
		p++;

	if (*p == '\n') {
		*p = '\0';				/* terminate this line */
		*cursor = p + 1;			/* next call starts after */
	} else {
		*cursor = p;				/* hit end of buffer */
	}

	return start;
}
