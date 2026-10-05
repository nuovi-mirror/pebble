#include <stdio.h>
#include <stdlib.h>

void error 
(char *msg)
{
	printf("error: %s\n", msg);
	exit(1);
}

void usage 
(void)
{
	printf("usage: rockc [ install | remove ] (pkg)\n");
	printf("    install - install package from package file (pkg)\n");
	printf("    remove  - remove installed package (pkg)\n");
	exit(1);
}
