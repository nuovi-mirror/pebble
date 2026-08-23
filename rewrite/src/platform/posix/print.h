#pragma once

#include <unistd.h>
#include "getstrlen.h"

void print(char *msg) {
	write(1, msg, getstrlen(msg));
}
