#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

typedef enum {
	type_word,
	type_sword,
	type_flt,
	type_str,
} ValueTypes;

typedef struct Value {
	ValueTypes Type;
	union {
		unsigned long word;
		long sword;
		double flt;
		char *str;
	}as;
} Value;

int valuetostr(char *buff, unsigned long buffsize, Value v) {
	switch (v.Type) {
		case type_word:
			return snprintf(buff, buffsize, "%lu", v.as.word);
		case type_sword:
			return snprintf(buff, buffsize, "%ld", v.as.sword);
		case type_flt:
			return snprintf(buff, buffsize, "%lf", v.as.flt);
		case type_str:
			return snprintf(buff, buffsize, "%s", v.as.str);
		default:
			return -1;
	}
}
			
Value guessvaluetype(const char *data) {
	Value out = { 0 };
	int consumed;

	if (data[0] == '-') {
		long value;

		/* check of sword */
		if (sscanf(data, "%ld%n", &value, &consumed) == 1 &&
				data[consumed] == '\0') {
			out.Type = type_sword;
			out.as.word = strtol(data, NULL, 10);
			return out;
		}
	} else {
		unsigned long value;

		/* check if word */
		if (sscanf(data, "%lu%n", &value, &consumed) == 1 &&
				data[consumed] == '\0') {
			out.Type = type_word;
			out.as.sword = strtoul(data, NULL, 10);
			return out;
		}
	}

	double value;

	/* check if float */
	if (sscanf(data, "%lf%n", &value, &consumed) == 1 &&
			data[consumed] == '\0') {
		out.Type = type_flt;
		out.as.flt = strtod(data, NULL);
		return out;
	}

	/* assume string after this point since all others fail */
	out.Type = type_str;
	out.as.str = data;
	return out;
}
