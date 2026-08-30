#pragma once

int cmpstrn
(const char *str1, const char *str2, unsigned long count) 
{
	unsigned long i = 0;

	while (i < count)
	{
		if (str1[i] != str2[i]) 
			return (unsigned char)str1[i] - (unsigned char)str2[i];

		if (str1[i] == '\0')
			return 0;

		i++;
	}

	return 0;
}
