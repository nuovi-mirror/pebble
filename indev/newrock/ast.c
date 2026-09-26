#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *mkast (char *src, unsigned long srcsize) {
	for (unsigned long i = 0; i < srcsize; i++)
		if (src[i] == '\n')
			src[i] = '\0';



}
