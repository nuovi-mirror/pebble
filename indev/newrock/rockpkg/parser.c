#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "helpers.h"
#include "globals.h"

static char *trim
(char *s)
{
    char *end;

    while (isspace((unsigned char)*s)) s++; /* skip leading whitespace */
    end = s + strlen(s); /* remove */

    while (end > s && isspace((unsigned char)end[-1])) end--;
    *end = '\0';

    return s;
}

void parsefile
(char *file, unsigned long size)
{
    char *line = file;
    char *end;

    (void)size;

    while (*line != '\0')
    {
        end = strchr(line, '\n'); /* find the end of this line */

        if (end != NULL) *end = '\0';
        line = trim(line); /* remove whitespace around the line */

        /* ignore empty lines */
        if (*line != '\0')
        {
            char *equals = strchr(line, '=');

            if (equals != NULL)
            {
                char *key;
                char *value;

                *equals = '\0';

                key = trim(line);
                value = trim(equals + 1);

		/* special case */
                if (strncmp(key, "src ", 4) == 0)
                {
                    char *srcfile;
                    char *url;

                    srcfile = trim(key + 4);

                    if (value[0] == '[') value++;
                    value = trim(value);

                    if (value[0] != '\0' && value[strlen(value) - 1] == ']') value[strlen(value) - 1] = '\0';
                    url = trim(value);

                    printf("source: %s -> %s\n", srcfile, url);
                }

                else if (strcmp(key, "name") == 0) name = value;
                else if (strcmp(key, "descr") == 0) descr = value;
                else if (strcmp(key, "longdescr") == 0) longdescr = value;
                else if (strcmp(key, "version") == 0) version = value;
                else if (strcmp(key, "contact") == 0) contact = value;
                else if (strcmp(key, "contacttype") == 0) contacttype = value;
                else if (strcmp(key, "program") == 0) program = value;
                else if (strcmp(key, "includedir") == 0) includedir = value;
                else if (strcmp(key, "output") == 0) output = value;
            }
        }

        if (end == NULL) break;
        line = end + 1;
    }
}
