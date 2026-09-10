#ifndef PLATFORM_PRINT_H_
#define PLATFORM_PRINT_H_

#include <unistd.h>
#include "getstrlen.h"

void print(char *msg) {
	write(1, msg, getstrlen(msg));
}

#endif
