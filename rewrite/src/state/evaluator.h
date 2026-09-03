#ifndef STATE_EVALUATOR_H_
#define STATE_EVALUATOR_H_

#include "values.h"
#include "expressions.h"
#include "variables.h"

#include "../platform/use/cmpstr.h"

Value evalexprdata
(ExprNodeData data, VarMap *vars);

Value resolveleaf
(Value v, VarMap *vars)
{
	if (v.Type != type_str)
		return v;

	Value *stored = getVar(vars, v.as.str);
	return stored != NULL ? *stored : v;
}

static ValueTypes numtype
(Value l, Value r)
{
	if (l.Type == type_flt || r.Type == type_flt)
		return type_flt;

	if (l.Type == type_sword || r.Type == type_sword)
		return type_sword;

	return type_word;
}

static long numsword
(Value v)
{ return v.Type == type_sword ? v.as.sword : (long)v.as.word; }

static unsigned long numword
(Value v)
{ return v.as.word; }

static double numflt
(Value v)
{
	switch (v.Type)
	{
		case type_flt:   return v.as.flt;
		case type_sword: return (double)v.as.sword;
		default:         return (double)v.as.word;
	}
}

static Value h_NumericAdd
(Value l, Value r)
{
	Value v;

	switch (numtype(l, r))
	{
		case type_flt: v.Type = type_flt; v.as.flt = numflt(l) + numflt(r); break;
		case type_sword: v.Type = type_sword; v.as.sword = numsword(l) + numsword(r); break;
		default: v.Type = type_word; v.as.word = numword(l) + numword(r); break;
	}

	return v;
}

static Value h_NumericSub
(Value l, Value r)
{
	Value v;

	switch (numtype(l, r))
	{
		case type_flt: v.Type = type_flt; v.as.flt = numflt(l) - numflt(r); break;
		case type_sword: v.Type = type_sword; v.as.sword = numsword(l) - numsword(r); break;
		default: v.Type = type_word; v.as.word = numword(l) - numword(r); break;
	}

	return v;
}

static Value h_NumericMul
(Value l, Value r)
{
	Value v;

	switch (numtype(l, r))
	{
		case type_flt: v.Type = type_flt; v.as.flt = numflt(l) * numflt(r); break;
		case type_sword: v.Type = type_sword; v.as.sword = numsword(l) * numsword(r); break;
		default: v.Type = type_word; v.as.word = numword(l) * numword(r); break;
	}

	return v;
}

static Value h_NumericDiv
(Value l, Value r)
{
	Value v;

	switch (numtype(l, r))
	{
		case type_flt: v.Type = type_flt; v.as.flt = numflt(r) == 0.0 ? 0.0 : numflt(l) / numflt(r); break;
		case type_sword: v.Type = type_sword; v.as.sword = numsword(r) == 0 ? 0 : numsword(l) / numsword(r); break;
		default: v.Type = type_word; v.as.word = numword(r) == 0 ? 0 : numword(l) / numword(r); break;
	}

	return v;
}

static int h_NumericCmp
(Value l, Value r)
{
	switch (numtype(l, r))
	{
		case type_flt: return numflt(l) > numflt(r) ? 1 : numflt(l) < numflt(r) ? -1 : 0;
		case type_sword: return numsword(l) > numsword(r) ? 1 : numsword(l) < numsword(r) ? -1 : 0;
		default: return numword(l) > numword(r) ? 1 : numword(l) < numword(r) ? -1 : 0;
	}
}

static const char *asstr
(Value v, char *buf, unsigned long n)
{
	if (v.Type == type_str)
		return v.as.str;

	valuetostr(buf, n, v);
	return buf;
}

static Value mkbool
(int b)
{
	Value v;
	v.Type = type_word;
	v.as.word = b != 0;
	return v;
}

Value evalexprnode
(ExprNode *node, VarMap *vars)
{
	Value l = evalexprdata(node->left, vars);
	Value r = evalexprdata(node->right, vars);
	char lb[64], rb[64];
	int cmp;

	switch (node->Op)
	{
		case ExprOp_NumericAdd: return h_NumericAdd(l, r);
		case ExprOp_NumericSub: return h_NumericSub(l, r);
		case ExprOp_NumericMul: return h_NumericMul(l, r);
		case ExprOp_NumericDiv: return h_NumericDiv(l, r);

		case ExprOp_NumericGreaterThan:
			cmp = h_NumericCmp(l, r);
			return mkbool(cmp > 0);

		case ExprOp_NumericLessThan:
			cmp = h_NumericCmp(l, r);
			return mkbool(cmp < 0);

		case ExprOp_NumericEqualTo:
			cmp = h_NumericCmp(l, r);
			return mkbool(cmp == 0);

		case ExprOp_NumericNotEqualTo:
			cmp = h_NumericCmp(l, r);
			return mkbool(cmp != 0);

		case ExprOp_StringEqualTo:
			return mkbool(
				cmpstr(
					asstr(l, lb, sizeof(lb)),
					asstr(r, rb, sizeof(rb))
				) == 0
			);

		case ExprOp_StringContains:
		case ExprOp_StringStartsWith:
		case ExprOp_StringEndsWith:
		{
			const char *ls = asstr(l, lb, sizeof(lb));
			const char *rs = asstr(r, rb, sizeof(rb));
			unsigned long ll = getstrlen(ls);
			unsigned long rl = getstrlen(rs);

			if (rl > ll)
				return mkbool(0);

			unsigned long start = 0;
			unsigned long end = ll - rl;

			if (node->Op == ExprOp_StringStartsWith) 
				end = 0;

			if (node->Op == ExprOp_StringEndsWith)
				start = end;

			for (unsigned long i = start; i <= end; i++)
				if (cmpstrn(ls + i, rs, rl) == 0)
					return mkbool(1);

			return mkbool(0);
		}

		case ExprOp_StringConcat:
		{
			const char *ls = asstr(l, lb, sizeof(lb));
			const char *rs = asstr(r, rb, sizeof(rb));
			unsigned long ll = getstrlen(ls);
			unsigned long rl = getstrlen(rs);
			char *out = lalloc(ll + rl + 1);

			copymem(out, ls, ll);
			copymem(out + ll, rs, rl);
			out[ll + rl] = '\0';

			Value v;
			v.Type = type_str;
			v.as.str = out;
			return v;
		}
	}

	return mkbool(0);
}

Value evalexprdata
(ExprNodeData data, VarMap *vars)
{
	if (data.Type == ExprDataVal)
		return resolveleaf(data.value, vars);

	return evalexprnode(data.Node, vars);
}

Value evalstr
(const char *str, int *ok, VarMap *vars, Arena *arena)
{ return evalexprdata(str2expr(str, ok, arena), vars); }

#endif
