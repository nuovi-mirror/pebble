#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#define PATH_MAX 32

void error 
(char *msg)
{
	printf("error: %s\n", msg);
	exit(1);
}

void usage 
(void)
{
	printf("usage: rockc [ install | remove ] (pkg)\n");
	printf("    install - install package from package file (pkg)\n");
	printf("    remove  - remove installed package (pkg)\n");
	exit(1);
}

int mkdir_p
(const char *path, mode_t mode)
{
	char tmp[PATH_MAX];
	size_t len = strlen(path);

	if (len >= sizeof tmp) return -1;

	memcpy(tmp, path, len + 1);

	while (len > 1 && tmp[len - 1] == '/') {
		tmp[--len] = '\0';
	}

	for (char *p = tmp + 1; *p; p++) {
		if (*p != '/') continue;

        	*p = '\0';

        	if (mkdir(tmp, mode) == -1 && errno != EEXIST) return -1;

        	*p = '/';
    	}

    	if (mkdir(tmp, mode) == -1 && errno != EEXIST) return -1;

    	return 0;
}
