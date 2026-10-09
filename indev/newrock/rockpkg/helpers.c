#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#define PATH_MAX 512

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
	printf("    remove  - remove installed package file (pkg)\n");
	printf("\n");
	printf("    (pkg)   - path to New Rock Package File (.rockpkg) file");
	exit(1);
}

int mkdir_p
(const char *path, mode_t mode)
{
	char tmp[PATH_MAX];

	size_t len = strlen(path);
	if (len >= sizeof tmp) return -1;

	memcpy(tmp, path, len + 1);
	while (len > 1 && tmp[len - 1] == '/') tmp[--len] = '\0';

	for (char *p = tmp + 1; *p; p++) 
	{
		if (*p != '/') continue;
        	*p = '\0';

        	if (mkdir(tmp, mode) == -1 && errno != EEXIST) return -1;
        	*p = '/';
    	}

    	if (mkdir(tmp, mode) == -1 && errno != EEXIST) return -1;

    	return 0;
}

int rmdir_p(const char *path)
{
    DIR *dir;
    struct dirent *entry;
    char tmp[PATH_MAX];
    int removed = 0;

    dir = opendir(path);

    if (!dir) 
    {
        if (rmdir(path) == 0) return 1;
        return -1;
    }

    while ((entry = readdir(dir)) != NULL) 
    {
        struct stat st;

        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;
        snprintf(tmp, sizeof(tmp), "%s/%s", path, entry->d_name);

        if (stat(tmp, &st) == -1) continue;
        if (S_ISDIR(st.st_mode)) 
	{
            int n = rmdir_p(tmp);

            if (n < 0) 
	    {
                closedir(dir);
                return -1;
            }

            removed += n;
        } else 
	{
            if (unlink(tmp) == 0) removed++;
            else 
	    {
                closedir(dir);
                return -1;
            }
        }
    }

    closedir(dir);

    if (rmdir(path) == 0) removed++;
    else return -1;

    return removed;
}
