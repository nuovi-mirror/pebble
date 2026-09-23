#include "values.h"
#include "expressions.h"
#include "variables.h"
#include "allocator.h"

Value evalexprdata
(ExprNodeData data, VarMap *vars, Arena *persistAlloc);

Value resolveleaf
(Value v, VarMap *vars);
Value evalexprnode

(ExprNode *node, VarMap *vars, Arena *persistAlloc);

Value evalexprdata
(ExprNodeData data, VarMap *vars, Arena *persistAlloc);

Value evalstr
(const char *str, int *ok, VarMap *vars, Arena *tempAlloc, Arena *persistAlloc);
