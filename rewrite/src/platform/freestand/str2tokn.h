#pragma once

char *str2tokn
(char *str, const char sep)
{
	static char *last;
	return str2toknl(str, sep, &last);
	/* and we are done */
}


char *str2toknl
(char *str, const char sep, char **last)
{
	if (str == NULL)
	{
		str = *last;

		if (str == NULL)
			return NULL;
	}

	/* skip deliminators */
	while (*str == sep)
		str++;

	/* nothing left to do */
	if (*str == '\0')
	{
		*last = NULL;
		return NULL;
	}

	token = str;

	/* find the end of the token */
	while (*str != sep && *str != '\0')
		str++;

	/* terminate the token and save the next position */
	if (*str == sep)
	{
		*str = '\0';
		*last = str + 1;
	}
	else 
	{
		*last = NULL;
	}

	return token;
}
