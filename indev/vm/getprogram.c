#include "readfile.h"
#include "main.h"
#include "allocator.h"

char *getprogram
(Arena *allocator, unsigned long maxsize, Args *cliargs)
{
	if (cliargs->count < 1) error(errfatal, errcodeplatformspecific, "No bytecode file specified!");

	unsigned long filesize = getfilesize(cliargs->values[1]);
	if (filesize == 0) error(errfatal, errcodeplatformspecific, "Cannot open specified bytecode file!");

	if (filesize + 1 > maxsize) error(errfatal, errcodelimitreached, "Bytecode file is too large!");

	char *file = alloc(allocator, filesize + 1);
	char *filedata = readfile(cliargs->values[1], file, filesize + 1);

	if (filedata == NULL) error(errfatal, errcodeplatformspecific, "Cannot open specified bytecode file!");

	return file;
}
