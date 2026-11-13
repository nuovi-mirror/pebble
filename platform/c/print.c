#include "print.h"
#include <stdio.h>

void print 
(const char *msg) 
{ 
	setbuf(stdout, NULL);
	fputs(msg, stdout); 
}
