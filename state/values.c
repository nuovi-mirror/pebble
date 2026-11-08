#include "values.h"
#include "allocator.h"
#include "copymem.h"
#include "copystr.h"
#include "evaluator.h"
#include "getstrlen.h"
#include "main.h"
#include "snprint.h"
#include "str2ul.h"
#include "str2l.h"
#include "str2d.h"
#include "trimflt.h"
#include "variables.h"
#include "limits.h"

#include <stdlib.h>

long numsword 
(Value v)
{
	switch (v.Type)
	{
		case type_sword: return v.as.sword; break;
		case type_word: 
			if (v.as.word > (unsigned long)LONG_MAX) error(errfatal, errcodenumericoverflow, "Overflow converting word to signed word!");
			return (long)v.as.word; break;
		case type_flt: 
			if (v.as.flt > (double)LONG_MAX) error(errfatal, errcodenumericoverflow, "Overflow converting float to signed word!");
			if (v.as.flt < LONG_MIN) error(errfatal, errcodenumericunderflow, "Underflow converting float to signed word!");
			return (long)v.as.flt; break;
		case type_str: return str2l(v.as.str); break; /* XXX legacy compat */
		case type_expr: return 0; break; /* XXX should never be hit */
		case type_pointer: return 0; break; /* XXX should never be hit */
		case type_null: return 0; break; /* XXX should never be hit */
		default: return 0; break; /* XXX should never be hit */
	}
}

unsigned long numword 
(Value v)
{
	switch (v.Type)
	{
		case type_word: return v.as.word; break;
		case type_sword: 
			if (v.as.sword < 0) error(errfatal, errcodenumericunderflow, "Underflow converting signed word to word!");
			return (unsigned long)v.as.word; break;
		case type_flt: 
			if (v.as.flt > (double)ULONG_MAX) error(errfatal, errcodenumericoverflow, "Overflow converting float to word!");
			if (v.as.flt < 0) error(errfatal, errcodenumericunderflow, "Underflow converting float to word!");
			return (unsigned long)v.as.flt; break;
		case type_str: return str2ul(v.as.str); break; /* XXX legacy compat */
		case type_expr: return 0; break; /* XXX should never be hit */
		case type_pointer: return 0; break; /* XXX should never be hit */
		case type_null: return 0; break; /* XXX should never be hit */
		default: return 0; break; /* XXX should never be hit */
	}
}

double numflt 
(Value v)
{
	char *end;
	switch (v.Type) 
	{
		case type_flt: return v.as.flt; break;
		case type_sword: return (double)v.as.sword; break;
		case type_word: return (double)v.as.word; break;
		case type_str: return str2d(v.as.str, &end); break; /* XXX legacy compat */
		case type_null: return 0; break; /* XXX should never be hit */
		case type_pointer: return 0; break; /* XXX should never be hit */
		case type_expr: return 0; break; /* XXX should never be hit */
		default: return 0.0; /* XXX error case */
	}
}

int valuetostr 
(char *buff, unsigned long buffsize, Value v, struct Limits *limits)
{
	if (buffsize > limits->misc_strlen) error(errfatal, errcodelimitreached, "String is too long!");

	switch (v.Type) 
	{
		case type_word: return snprint(buff, buffsize, "%lu", v.as.word);
		case type_sword: return snprint(buff, buffsize, "%ld", v.as.sword);
		case type_flt: 
		{
			int n = snprint(buff, buffsize, "%lf", v.as.flt);

			if (n <= 0 || buffsize == 0) return n;
			if ((unsigned long)n >= buffsize) n = (int)(buffsize - 1); /* output was truncated */

			return (int)trimflt(buff, (unsigned long)n);
		}
		case type_str: return snprint(buff, buffsize, "%s", v.as.str);
		case type_null: if (buffsize) copystr(buff, "NULL"); return 0;
		case type_pointer: if (buffsize) copystr(buff, "NULL"); return 0;
		case type_expr:
			if (v.as.expr == NULL || v.as.expr->Source == NULL) {
				if (buffsize) buff[0] = '\0'; /* never leave buff unterminated */
				return -1;
			}
			return snprint(buff, buffsize, "%s", v.as.expr->Source);

		default:
			if (buffsize)
				buff[0] = '\0'; /* never leave buff unterminated */
			return -1;		/* should never be hit */
	}
}

Value valuetoword 
(Value v, VarMap *vars, Arena *persistAlloc, struct Limits *limits)
{
	switch (v.Type) {
		case type_sword: /* XXX shared */
		case type_word: /* XXX shared */
		case type_flt: return (Value){ .Type = type_word, .as.word = numword(v) };
		case type_str: return (Value){ .Type = type_word, .as.word = str2ul(v.as.str) };
		case type_expr: return evalexprnode(v.as.expr, vars, persistAlloc, limits);
		default: return (Value){ .Type = type_word, .as.word = 0 };
	}
}

Value valuetosword 
(Value v, VarMap *vars, Arena *persistAlloc, struct Limits *limits)
{
	switch (v.Type) {
		case type_sword: /* XXX shared */
		case type_word: /* XXX shared */
		case type_flt: return (Value){ .Type = type_sword, .as.sword = numsword(v) };
		case type_str: return (Value){ .Type = type_sword, .as.sword = str2l(v.as.str) };
		case type_expr: return evalexprnode(v.as.expr, vars, persistAlloc, limits);
		default: return (Value){ .Type = type_sword, .as.sword = 0 };
	}
}

Value guessvaluetype 
(char *data, Arena *persistAlloc)
{
	Value out = {0};
	unsigned long i = 0;
	int negative = 0;
	int dot = 0;

	if (data[0] == '\0') goto string; /* empty string */

	/* check if contains valid numeric chars */
	char *p = data;

	while (*p)
	{
		unsigned char c = (unsigned char)*p++;
		if (!((c >= '0' && c <= '9') || c == '-' || c == '.')) goto string;
	}

	/* optional negative sign */
	if (data[0] == '-') {
		negative = 1;
		i++;

		if (data[i] == '\0') goto string; /* '-' by itself */
		if (data[1] == '0' && data[2] != '.') goto string; /* -0 */
	}

	/* validate the numeric form */
	for (; data[i] != '\0'; i++) {
		if (data[i] == '.') {
			if (dot) goto string;

			dot = 1;
			continue;
		}

		if (data[i] < '0' || data[i] > '9') goto string;
	}

	/* leading 0 with no decimal is a string so things like '01' are preserved. */
	if (data[0] == '0' && !dot && i > 1) goto string;

	/* decimal number */
	if (dot) 
	{
		out.Type = type_flt;
		out.as.flt = strtod(data, NULL);
		return out;
	}

	/* negative integer */
	if (negative) 
	{
		out.Type = type_sword;
		out.as.sword = strtol(data, NULL, 10);
		return out;
	}

	/* positive integer */
	out.Type = type_word;
	out.as.word = strtoul(data, NULL, 10);
	return out;

string:
	out.Type = type_str;
	unsigned long len = getstrlen(data);
	out.as.str = alloc(persistAlloc, len + 1);
	copymem(data, out.as.str, len + 1);
	return out;
}

