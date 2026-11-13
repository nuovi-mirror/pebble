#ifndef h_getprogram_
#define h_getprogram_

typedef struct Program {
	unsigned long size;
	char *data;
} Program;

Program getprogram (unsigned long maxsize, Args *cliargs);

#endif
