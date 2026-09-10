#ifndef STATE_VALUES_H_
#define STATE_VALUES_H_

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

#include "../platform/use/copystr.h"

typedef struct ExprNode ExprNode; /* XXX hack */

typedef enum {
	type_word,
	type_sword,
	type_flt,
	type_str,
	type_expr,
	type_null, /* debug */
} ValueTypes;

typedef struct Value {
	ValueTypes Type;
	union {
		unsigned long word;
		long sword;
		double flt;
		char *str;
		ExprNode *expr;
		void *null;
	}as;
} Value;

/* moved to expressions.h */
int valuetostr(char *buff, unsigned long buffsize, Value v);

Value guessvaluetype(char *data) {
	Value out = { 0 };
	int consumed;

	if (data[0] == '-') {
		long value;

		/* check of sword */
		if (sscanf(data, "%ld%n", &value, &consumed) == 1 &&
				data[consumed] == '\0') {
			out.Type = type_sword;
			out.as.sword = strtol(data, NULL, 10);
			return out;
		}
	} else {
		unsigned long value;

		/* check if word */
		if (sscanf(data, "%lu%n", &value, &consumed) == 1 &&
				data[consumed] == '\0') {
			out.Type = type_word;
			out.as.word = strtoul(data, NULL, 10);
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

#endif
