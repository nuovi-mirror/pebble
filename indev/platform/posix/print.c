#include <unistd.h>
#include "getstrlen.h"
#include "print.h"

void print(char *msg) {
	write(1, msg, getstrlen(msg));
}
