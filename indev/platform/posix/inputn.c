#include "inputn.h"
#include "main.h"
#include <termios.h>
#include <unistd.h>

unsigned long inputn (char *buff, unsigned long size) {
	if (size == 0) return 0;
	if (buff == NULL) return 0;

	struct termios told;
	struct termios tnew;
	unsigned long i = 0;
	int ch;

	if (tcgetattr(0, &told) == -1)
		return 0;

	tnew = told;
	tnew.c_lflag &= ~(ICANON | ECHO);
	tnew.c_cc[VMIN] = 1;
	tnew.c_cc[VTIME] = 0;

	if (tcsetattr(0, TCSANOW, &tnew) == -1) 
		return 0;

	while (i < size && read(0, &ch, 1) == 1) 
		buff[i++] = (char)ch;

	tcsetattr(0, TCSANOW, &told);

	/* buff[i] = '\0'; XXX no end char */
	return i;
}
