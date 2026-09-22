#ifndef STATE_VALUES_H_
#define STATE_VALUES_H_

#include "variables.h"
#include "allocator.h"

typedef struct ExprNode ExprNode; /* from expressions.h */

typedef enum {
	type_word,
	type_sword,
	type_flt,
	type_str,
	type_expr,
	type_null, /* debug */
	type_pointer, /* debug */
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
		void *pointer;
	}as;
} Value;

int valuetostr(char *buff, unsigned long buffsize, Value v);
Value valuetoword(Value v, VarMap *vars, Arena *persistAlloc);
Value guessvaluetype(char *data, Arena *persistAlloc);

#endif
