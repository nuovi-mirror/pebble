#ifndef STATE_EVALUATOR_H_
#define STATE_EVALUATOR_H_

#include "values.h"
#include "expressions.h"
#include "variables.h"

#include "../platform/use/cmpstr.h"

Value evalexprdata(ExprNodeData data, VarMap *vars);

Value resolveleaf(Value v, VarMap *vars)
{
	if (v.Type != type_str)
		return v;

	Value *stored = getVar(vars, v.as.str);
	return stored != NULL ? *stored : v; /* not known var, return varname text */
}

static double asnum(Value v)
{
	switch (v.Type)
	{
		case type_word: return (double)v.as.word;
		case type_sword: return (double)v.as.sword;
		case type_flt: return v.as.flt;
		default: return 0.0;
	}
}

static const char *asstr(Value v, char *buf, unsigned long n)
{
	if (v.Type == type_str) return v.as.str;
	valuetostr(buf, n, v);
	return buf;
}

static Value mknum(double d)  { Value v; v.Type = type_flt; v.as.flt = d;         return v; }
static Value mkbool(int b)    { Value v; v.Type = type_word; v.as.word = b != 0;  return v; }

Value evalexprnode(ExprNode *node, VarMap *vars)
{
	Value l = evalexprdata(node->left, vars);
	Value r = evalexprdata(node->right, vars);
	char lb[64], rb[64];

	switch (node->Op)
	{
		case ExprOp_NumericAdd: return mknum(asnum(l) + asnum(r));
		case ExprOp_NumericSub: return mknum(asnum(l) - asnum(r));
		case ExprOp_NumericMul: return mknum(asnum(l) * asnum(r));
		case ExprOp_NumericDiv: return asnum(r) == 0 ? mknum(0) : mknum(asnum(l) / asnum(r));

		case ExprOp_NumericGreaterThan: return mkbool(asnum(l) > asnum(r));
		case ExprOp_NumericLessThan:    return mkbool(asnum(l) < asnum(r));
		case ExprOp_NumericEqualTo:     return mkbool(asnum(l) == asnum(r));
		case ExprOp_NumericNotEqualTo:  return mkbool(asnum(l) != asnum(r));

		case ExprOp_StringEqualTo:
			return mkbool(cmpstr(asstr(l, lb, sizeof(lb)), asstr(r, rb, sizeof(rb))) == 0);

		case ExprOp_StringContains:
		case ExprOp_StringStartsWith:
		case ExprOp_StringEndsWith:
		{
			const char *ls = asstr(l, lb, sizeof(lb));
			const char *rs = asstr(r, rb, sizeof(rb));
			unsigned long ll = getstrlen(ls), rl = getstrlen(rs);
			if (rl > ll) return mkbool(0);

			unsigned long start = 0, end = ll - rl;
			if (node->Op == ExprOp_StringStartsWith) end = 0;
			if (node->Op == ExprOp_StringEndsWith)    start = end;

			for (unsigned long i = start; i <= end; i++)
				if (cmpstrn(ls + i, rs, rl) == 0)
					return mkbool(1);
			return mkbool(0);
		}

		case ExprOp_StringConcat:
		{
			const char *ls = asstr(l, lb, sizeof(lb));
			const char *rs = asstr(r, rb, sizeof(rb));
			unsigned long ll = getstrlen(ls), rl = getstrlen(rs);
			char *out = lalloc(ll + rl + 1);
			copymem(out, ls, ll);
			copymem(out + ll, rs, rl);
			out[ll + rl] = '\0';
			Value v; v.Type = type_str; v.as.str = out;
			return v;
		}
	}

	return mknum(0); /* unreachable */
}

Value evalexprdata(ExprNodeData data, VarMap *vars)
{
	if (data.Type == ExprDataVal)
		return resolveleaf(data.value, vars);

	return data.Type == ExprDataVal ? data.value : evalexprnode(data.Node, vars);
}

Value evalstr(const char *str, int *ok, VarMap *vars)
{
	return evalexprdata(str2expr(str, ok), vars);
}

#endif
