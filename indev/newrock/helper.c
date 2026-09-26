#include "helper.h"
#include <stdio.h>
#include <stdlib.h>

void error (int err, const char *msg) {
	switch (err) {
		case error_fatal_int: 
			printf("%s: %s\n", error_fatal_str, msg); 
			exit(error_fatal_int); 
			break;

		case error_warning_int: 
			printf("%s: %s\n", error_warning_str, msg); 
			exit(error_warning_int); 
			break;
	}
}

char *readfile (const char *path, char *buff, unsigned long size) {
	if (path == NULL) error(1, "no file specified");
	
	FILE *file = fopen(path, "rb");
	if (!file) error(1, "cannot open file");
	
	unsigned long read = fread(buff, size, 1, file);
	if (read == 0) return NULL;
	
	((char *)buff)[read] = '\0';
	return buff;
}
