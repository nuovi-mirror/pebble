#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "helpers.h"
#include "globals.h"
#include "stuff.h"
	
#define filesize 512
#define filebuffsize 12582912

int main
(int argc, char **argv)
{
	if (argc < 2) usage();
	
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

	char bbasedir[32];
	char bpkgdir[sizeof(bbasedir) + 32];
	char bappdir[sizeof(bpkgdir) + 32];

	char *basedir = (char *)&bbasedir;
	char *pkgdir = (char *)&bpkgdir;
	char *appdir = (char *)&bappdir;

	const char *home = getenv("HOME");
	snprintf(basedir, sizeof(bbasedir), "%s%s", home, "/.rock/");
	snprintf(pkgdir, sizeof(bpkgdir), "%s%s", basedir, "apps/");

	mkdir_p(basedir, 0755); 
	mkdir_p(pkgdir, 0755); 

	FILE *file = fopen(argv[2], "r");
	if (file == NULL) error("cannot open file");

	char *filedata = malloc(filesize + 1);
	if (filedata == NULL) error("cannot allocate file buffer");
	fread(filedata, filesize, 1, file);
	fclose(file);

	filedata[filesize] = '\0';	

	parsefile(filedata, filesize);

	snprintf(appdir, sizeof(bappdir), "%s/%s/%s/", pkgdir, name, version);

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

		char chdirbuff[512];
		snprintf(chdirbuff, 512, "%s", appdir);
		mkdir_p(chdirbuff, 0755);

		if ((chdir(chdirbuff)) == -1) error("cannot enter dir");
		mkdir_p("bin", 0755);
		mkdir_p("../../../bin", 0755);

		for (unsigned long i = 0; i < srccount; i++)
		{
			char *buff = malloc(filebuffsize);
			if (buff == NULL) error("failed to allocate buffer");

			char path[512];
			snprintf(path, sizeof(path), "%s", srcs[i]);

			char *slash = strrchr(path, '/');
			if (slash != NULL)
			{
				*slash = '\0';

				if (mkdir_p(path, 0755) == -1) error("cannot create source directory");
			}

			printf("Fetching %s to %s\n", srcurls[i], srcs[i]);
			unsigned long size = fetch(buff, filebuffsize, srcurls[i]);

			FILE *file = fopen(srcs[i], "w");
			if (file == NULL) error("cannot open source file");

			if (fwrite(buff, 1, size, file) != size) error("failed to write to file");
			if (fclose(file) != 0) error("failed to close file");

			free(buff);
		}

		char outpath[512];
		char buff[512 + sizeof(outpath)];
		snprintf((char *)&outpath, sizeof(outpath), "bin/%s", name);
		snprintf((char *)&buff, 512, "rockc %s %s >%s", program, includedir, (char *)&outpath);

		printf("rockc: %s\n", buff);
		if ((system(buff)) != 0) error("compiling failed");

		if (chmod((char *)&outpath, 0555) == -1) error("cannot make program executable");

		char linkpath[512];
		char linkpath2[512];
		snprintf((char *)&linkpath, sizeof(linkpath), "%sbin/%s", basedir, name);
		snprintf((char *)&linkpath2, sizeof(linkpath2), "%sbin/%s", appdir, name);
		printf("Linking %s to %s\n", linkpath, linkpath2);
		remove(linkpath);
		if (symlink(linkpath2, linkpath) == -1) error("cannot symlink");

	} else if (strcmp(argv[1], "remove") == 0)
	{
		printf("remove function not implimented yet :(\n");
		exit(0);
	} else usage();
}
