#ifndef h_evaluator_
#define h_evaluator_
#include "allocator.h"
#include "expressions.h"
#include "values.h"
#include "variables.h"

Value evalexprdata (ExprNodeData data, VarMap *vars, Arena *persistAlloc);
Value resolveleaf (Value v, VarMap *vars);
Value evalexprnode (ExprNode *node, VarMap *vars, Arena *persistAlloc);

Value evalexprdata (ExprNodeData data, VarMap *vars, Arena *persistAlloc);
Value evalstr (const char *str, int *ok, VarMap *vars, Arena *tempAlloc,
	Arena *persistAlloc);
#endif
