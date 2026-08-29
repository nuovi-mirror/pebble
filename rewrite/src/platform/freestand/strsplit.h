#pragma once

void strsplit
(char *str, char delim)
{
	unsigned long i = 0;

	while (str[i] != '\0')
	{
		if (str[i] == delim)
			str[i] = '\0';

		i++;
	}
}
