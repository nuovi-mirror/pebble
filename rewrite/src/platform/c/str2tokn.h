#pragma once

/* okay i honestly have no idea how to make this function */
/* so we are just using libc */

char *str2tokn
(char *str, const char *sep)
{
	return strtok(str, sep);
}
