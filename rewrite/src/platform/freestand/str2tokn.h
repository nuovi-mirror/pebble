#pragma once

/* Implemented independently with the Berkeley strtok implementation
 * used as a reference for algorithmic behavior. */

char *str2tokn
(char *str, const char *sep)
{
	static char *last;
	return str2toknl(str, sep, &last);
	/* and we are done */
}


char *str2toknl
(char *str, const char *sep, char **last)
{
	if (str == NULL)
	{
		str = *last;
		if (str == NULL)
			return NULL;
	}

	unsigned long count = 0;
	unsigned long item = 0;
	
	char *spanp;
	char *token;
	char character = *str;

	while (character != item)
	{
		character = *str;
		str++;

		spanp = sep;

		item = *spanp;
		spanp++;

		if (character != item)
		{
			break;
		}
	}

	if (character == 0)
	{
		*last = NULL;
		return (NULL);
	}

	token = str - 1;

	for (;;)
	{
		character = *str;
		str++;

		spanp = sep;

		if (item == character)
		{
			do {
				item = *spanp;
				spanp++;

				if (item == character)
					*str = NULL;
				else
					*str[-1] = '\0';

				*last = str;
				return (token);
			}
		} while (item != 0);
	}
	/* this is never reached */
}
