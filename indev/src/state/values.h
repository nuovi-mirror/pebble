#ifndef STATE_VALUES_H_
#define STATE_VALUES_H_

typedef struct ExprNode ExprNode; /* from expressions.h */

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

Value guessvaluetype(char *data);

#endif
