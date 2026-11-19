#ifndef h_getprogram_
#define h_getprogram_

#include "main.h"

typedef struct Program {
	unsigned long size;
	unsigned long instructions;
	char *data;
} Program;

Program getprogram (unsigned long maxsize, Args *cliargs);

#endif
