#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "helpers.h"
#include "globals.h"
#include "stuff.h"
	
#define filesize 512

int main
(int argc, char **argv)
{
	
	name = malloc(32);
	descr = malloc(64);
	longdescr = malloc(512);
	version = malloc(32);
	contact = malloc(32);
	contacttype = malloc(32);
	srcs[maxsrcs];
	bsrcs[maxsrcs];
	program = malloc(32);
	includedir = malloc(32);
	output = malloc(32);

	if (argc < 2) usage();

	FILE *file = fopen(argv[2], "r");
	if (file == NULL) error("cannot open file");

	char *filedata = malloc(filesize + 1);
	fread(filedata, filesize, 1, file);

	filedata[filesize + 1] = '\0';	

	parsefile(filedata, filesize);

	if (strcmp(argv[1], "install") == 0)
	{
		printf("Package name: %s\n", name);
		printf("Package description: %s\n", descr); 
		printf("Package version: %s\n", version);
		printf("Contact: %s (%s)\n", contact, contacttype);
	} else if (strcmp(argv[1], "remove") == 0)
	{
		printf("remove function not implimented yet :(\n");
		exit(0);
	} else usage();
}
