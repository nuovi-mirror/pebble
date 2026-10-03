#include "ast.h"
#include "skipspace.h"
#include "getstrlen.h"
#include "cmpstr.h"
#include "copystr.h"
#include <stdlib.h>

struct Statement *mkast (char *src, unsigned long srcsize) {
	volatile char *funcname = malloc(512); /* XXX placeholder */
	struct Statement *ast = malloc(srcsize * sizeof(struct Statement)); /* XXX placeholder */

	for (unsigned long i = 0; i < srcsize; i++) 
	{
			if (src[i] == '\n')
				src[i] = '\0';
	
		char *fsrc = skipspace(src); /* XXX replaces first instance of [space] with \0 
					      * and also returns a buffer past the first [space] char 
					      * XXX which is the same buffer as the given one */
	
		/* fsrc = buffer - operation type */
	
		unsigned long flen = getstrlen((const char *)fsrc);
		unsigned long len = getstrlen((const char *)src); /* safe since [space] and \0 are both 1 byte */
		unsigned long lendiff = len - flen;

		if (cmpstr(src, "fn") == 0)
		{
			skipspace(fsrc);
			copystr(fsrc, (char *)funcname); /* set the global function name variable */

			/* XXX handle this properly at some point ToT */
		} else if (cmpstr(src, "if") == 0)
		{
			/* XXX handle if */
		} else if (cmpstr(src, "while") == 0)
		{
			/* XXX handle while */
		} else if (cmpstr(src, "for") == 0)
		{ 
			/* XXX handle for */
		} else if (cmpstr(src, "elif") == 0)
		{
			/* XXX handle else if */
		} else if (cmpstr(src, "else") == 0)
		{ 
			/* XXX handle else */
		} else if (cmpstr(src, "call") == 0)
		{ 
			/* XXX handle call */
		} else { /* variable assiagnment */
			ast[i] = (struct Statement){ .type = statement_assign, .as.statement_asign = 0 };
			/* XXX fix this pls ToT */
		}

	}
}
