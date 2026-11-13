#include "readfile.h"
#include "lalloc.h"
#include "main.h"
#include "getprogram.h"

Program getprogram
(unsigned long maxsize, Args *cliargs)
{
	if (cliargs->count <= 1) error(errfatal, errcodeplatformspecific, "No bytecode file specified!");

	unsigned long filesize = getfilesize(cliargs->values[1]);
	if (filesize == 0) error(errfatal, errcodeplatformspecific, "Cannot open specified bytecode file!");

	if (filesize + 1 > maxsize) error(errfatal, errcodelimitreached, "Bytecode file is too large!");

	char *file = lalloc(filesize + 1, 3);
	char *filedata = readfile(cliargs->values[1], file, filesize + 1);

	if (filedata == NULL) error(errfatal, errcodeplatformspecific, "Cannot open specified bytecode file!");

	Program program;
	program.data = file;
	program.size = filesize;

	return program;
}
