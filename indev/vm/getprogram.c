#include "readfile.h"
#include "main.h"
#include "print.h"
#include "exitproc.h"

unsigned long getprogram
(char *file, unsigned long maxsize, Args *cliargs)
{
	unsigned long filesize = getfilesize(cliargs->values[1]);
	if (filesize == 0) {
		print("ERROR: VM: INIT: CANNOT OPEN SPECIFIED FILE!\n");
		exitproc(1);
	}

	if (filesize + 1 > maxsize) {
		print("ERROR: VM: INIT: FILE IS TOO LARGE!\n");
		exitproc(1);
	}

	char *filedata = readfile(cliargs->values[1], file, filesize + 1);

	if (filedata == NULL) {
		print("ERROR: VM: INIT: CANNOT OPEN SPECIFIED FILE!\n");
		exitproc(1);
	}

	return filesize;
}
