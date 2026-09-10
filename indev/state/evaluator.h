#ifndef STATE_EVALUATOR_H_
#define STATE_EVALUATOR_H_

#include "values.h"
#include "expressions.h"
#include "variables.h"

Value evalexprdata
(ExprNodeData data, VarMap *vars);

Value resolveleaf
(Value v, VarMap *vars);

static ValueTypes numtype
(Value l, Value r);

static long numsword
(Value v);

static unsigned long numword
(Value v);

static double numflt
(Value v);

static Value h_NumericAdd
(Value l, Value r);

static Value h_NumericSub
(Value l, Value r);

static Value h_NumericMul
(Value l, Value r);

static Value h_NumericDiv
(Value l, Value r);

static int h_NumericCmp
(Value l, Value r);

static const char *asstr
(Value v, char *buf, unsigned long n);

static Value mkbool
(int b);

Value evalexprnode
(ExprNode *node, VarMap *vars);

Value evalexprdata
(ExprNodeData data, VarMap *vars);

Value evalstr
(const char *str, int *ok, VarMap *vars, Arena *arena);

#endif
