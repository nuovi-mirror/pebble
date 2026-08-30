#ifndef PLATFORM_SNPRINT_H_
#define PLATFORM_SNPRINT_H_

#include <stdio.h>
#include <stdarg.h>

int snprint
(char * restrict str, size_t size, const char * restrict format, ...)
{
	va_list args;
	va_start(args, format);

	int result = vsnprintf(str, size, format, args);

	va_end(args);
	return result;
}

#endif
