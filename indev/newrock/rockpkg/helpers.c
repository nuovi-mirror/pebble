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
	printf("put usage data here");
	exit(1);
}
