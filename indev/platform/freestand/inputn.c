#ifdef noinput
#define platform_inputn_

#include "inputn.h"
#include "main.h"

unsigned long inputn 
(char *buff, unsigned long size) {
	error(errwarning, errcodeundefined, "-Dnoinput was included; cannot get input!");
	buff[size] = '\0'; /* never leave the buffer unterminated */
	return 0;
}

#endif
