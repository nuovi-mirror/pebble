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

void printValue(Value v) {
	switch(v.Type) {
		case type_word:
			printf("word of %lu\n", v.as.word);
			break;
		case type_sword:
			printf("sword of %ld\n", v.as.sword);
			break;
		case type_flt:
			printf("float of %f\n", v.as.flt);
			break;
		case type_str:
			printf("string of \"%s\"\n", v.as.str);
			break;
	}
}

int valuetostr(char *buff, Value v) {
	switch (v.Type) {
		case type_word:
			return snprintf(buff, sizeof(buff), "%lu", v.as.word);
		case type_sword:
			return snprintf(buff, sizeof(buff), "%ld", v.as.sword);
		case type_flt:
			return snprintf(buff, sizeof(buff), "%f", v.as.flt);
		case type_str:
			return snprintf(buff, sizeof(buff), "%s", v.as.str);
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
			out.as.word = strtoul(data, NULL, 10);
			return out;
		}
	} else {
		unsigned long value;

		/* check if word */
		if (sscanf(data, "%lu%n", &value, &consumed) == 1 &&
				data[consumed] == '\0') {
			out.Type = type_word;
			out.as.sword = strtol(data, NULL, 10);
			return out;
		}
	}

	double value;

	/* check if float */
	if (sscanf(data, "%f%n", &value, &consumed) == 1 &&
			data[consumed] == '\0') {
		out.Type = type_flt;
		out.as.flt = strtof(data, NULL);
		return out;
	}

	/* assume string after this point since all others fail */
	out.Type = type_str;
	out.as.str = data;
	return out;
}
