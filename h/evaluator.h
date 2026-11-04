#ifndef h_evaluator_
#define h_evaluator_

#include "allocator.h"
#include "expressions.h"
#include "values.h"
#include "variables.h"
#include "limits.h"

Value evalexprdata (ExprNodeData data, VarMap *vars, Arena *persistAlloc, struct Limits *limits);
Value resolveleaf (Value v, VarMap *vars);
Value evalexprnode (ExprNode *node, VarMap *vars, Arena *persistAlloc, struct Limits *limits);

Value evalexprdata (ExprNodeData data, VarMap *vars, Arena *persistAlloc, struct Limits *limits);
Value evalstr (const char *str, int *ok, VarMap *vars, Arena *tempAlloc, Arena *persistAlloc, struct Limits *limits);

#endif
