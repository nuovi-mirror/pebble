#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include "copystr.h"
#include "snprint.h"
#include "str2ul.h"
#include "values.h"
#include "main.h"
#include "expressions.h"
#include "evaluator.h"
#include "variables.h"

int valuetostr(char *buff, unsigned long buffsize, Value v) {
	switch (v.Type) {
		case type_word:  return snprint(buff, buffsize, "%lu", v.as.word);
		case type_sword: return snprint(buff, buffsize, "%ld", v.as.sword);
		case type_flt:   return snprint(buff, buffsize, "%lf", v.as.flt);
		case type_str:   return snprint(buff, buffsize, "%s", v.as.str);
		case type_null:  if (buffsize) copystr(buff, "NULL"); return 0;
		case type_expr:  
			if (v.as.expr == NULL || v.as.expr->Source == NULL)
			{
				if (buffsize) buff[0] = '\0'; /* never leave buff unterminated */
				return -1;
			}
			return snprint(buff, buffsize, "%s", v.as.expr->Source);
		default:	 
			if (buffsize) buff[0] = '\0'; /* never leave buff unterminated */
			return -1; /* should never be hit */
	}
}

Value valuetoword(Value v, VarMap *vars) {
	switch (v.Type) {
		case type_word:  return v;
		case type_sword: return (struct Value){ type_word, (unsigned long)v.as.sword };
		case type_flt:   return (struct Value){ type_word, (unsigned long)v.as.flt };
		case type_str:   return (struct Value){ type_word, str2ul(v.as.str, NULL, 10) };
		case type_null:  return (struct Value){ type_word, 0 }; 
				 /* XXX  ¯\_(°▽°)_/¯  idk what to put here ngl */
		case type_expr:  
			return evalexprnode(v.as.expr, vars);
		default:	 
			return (struct Value){ type_word, -1 }; /* should never be hit */
	}
}

Value guessvaluetype
(char *data)
{
	Value out = { 0 };
	unsigned long i = 0;
	int negative = 0;
	int dot = 0;

	/* empty string */
	if (data[0] == '\0')
		goto string;

	/* optional negative sign */
	if (data[0] == '-')
	{
		negative = 1;
		i++;

		/* '-' by itself */
		if (data[i] == '\0')
			goto string;

		if (data[1] == '0')
			goto string;
	}

	/* validate the numeric form */
	for (; data[i] != '\0'; i++)
	{
		if (data[i] == '.')
		{
			/* only one decimal point */
			if (dot)
				goto string;

			dot = 1;
			continue;
		}

		/* anything other than a digit */
		if (data[i] < '0' || data[i] > '9')
			goto string;
	}

	/*
	 * Leading zero without a decimal point is a string.
	 * This makes things like "01" usable as identifiers.
	 */
	if (data[0] == '0' && !dot)
		goto string;

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
	out.as.str = data;
	return out;
}
