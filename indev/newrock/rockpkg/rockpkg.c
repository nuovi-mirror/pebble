#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "helpers.h"
#include "globals.h"
#include "stuff.h"
	
#define filesize 512
#define filebuffsize 12582912

int main
(int argc, char **argv)
{
	
	name = malloc(32);
	descr = malloc(64);
	longdescr = malloc(512);
	version = malloc(32);
	contact = malloc(32);
	contacttype = malloc(32);
	program = malloc(32);
	includedir = malloc(32);
	output = malloc(32);
	srccount = 0;

	for (unsigned long i = 0; i < maxsrcs; i++)
	{
		srcs[i] = malloc(32);
		srcurls[i] = malloc(512);
	}

	if (argc < 2) usage();

	FILE *file = fopen(argv[2], "r");
	if (file == NULL) error("cannot open file");

	char *filedata = malloc(filesize + 1);
	if (filedata == NULL) error("cannot allocate file buffer");
	fread(filedata, filesize, 1, file);

	filedata[filesize] = '\0';	

	parsefile(filedata, filesize);

	if (strcmp(argv[1], "install") == 0)
	{
		printf("Package name: %s\n", name);
		printf("Package description: %s\n", descr); 
		printf("Package version: %s\n", version);
		printf("Contact: %s (%s)\n", contact, contacttype);

		for (unsigned long i = 0; i < srccount; i++)
			printf("Source: %s -> %s\n", srcs[i], srcurls[i]);

		printf("Are you sure you wish to install this? (y/n): ");
		
		char input;
		scanf(" %c", &input);

		if (input != 'y') error("Aborted!");

		for (unsigned long i = 0; i < srccount; i++)
		{
			char *buff = malloc(filebuffsize);
			if (buff == NULL) error("failed to allocate buffer");

			unsigned long size = fetch(buff, filebuffsize, srcurls[i]);

			FILE *file = fopen(srcs[i], "w");
			if (file == NULL) error("cannot open source file");

			fwrite(buff, size, 1, file);

			free(buff);
		}

	} else if (strcmp(argv[1], "remove") == 0)
	{
		printf("remove function not implimented yet :(\n");
		exit(0);
	} else usage();
}
