#include "evaluator.h"
#include "cmpstr.h"
#include "cmpstrn.h"
#include "copymem.h"
#include "expressions.h"
#include "getnstrlen.h"
#include "getstrlen.h"
#include "str2d.h"
#include "str2ul.h"
#include "main.h"
#include "values.h"
#include "variables.h"
#include "strtokn.h"

#include <limits.h>
#include <math.h>
#include <float.h>

Value resolveleaf 
(Value v, VarMap *vars)
{
	if (v.Type != type_str) return v;

	Value *stored = getVar(vars, v.as.str);
	return stored != NULL ? *stored : v;
}

static ValueTypes numtype 
(Value l, Value r)
{
	if (l.Type == type_flt || r.Type == type_flt) return type_flt;
	if (l.Type == type_sword || r.Type == type_sword) return type_sword;
	return type_word;
}

static long numsword 
(Value v)
{ return v.Type == type_sword ? v.as.sword : (long)v.as.word; }

static unsigned long numword 
(Value v)
{
	char *end;

	switch (v.Type)
	{
		case type_word: return v.as.word; break;
		case type_sword: return (unsigned long)v.as.word; break;
		case type_flt: return (unsigned long)v.as.flt; break;
		case type_str: return str2ul(v.as.str, &end, 10); break;
		case type_expr: return 0; break; /* XXX should never be hit */
		case type_pointer: return 0; break; /* XXX should never be hit */
		case type_null: return 0; break; /* XXX should never be hit */
		default: return 0; break; /* XXX should never be hit */
	}
}

static double numflt 
(Value v)
{
	char *end;

	switch (v.Type) 
	{
		case type_flt: return v.as.flt; break;
		case type_sword: return (double)v.as.sword; break;
		case type_word: return (double)v.as.word; break;
		case type_str: return str2d(v.as.str, &end); break;
		case type_null: return 0; break; /* XXX should never be hit */
		case type_pointer: return 0; break; /* XXX should never be hit */
		case type_expr: return 0; break; /* XXX should never be hit */
		default: return 0.0; /* XXX error case */
	}
}

static Value h_NumericAdd 
(Value l, Value r)
{
	Value v;

	switch (numtype(l, r)) 
	{
		case type_flt: 
		{
			double lf = numflt(l);
			double rf = numflt(r);

			if (rf > 0.0 && lf > DBL_MAX - rf) error(errfatal, errcodenumericoverflow, "Floating addition positive overflow!");
			if (rf < 0.0 && lf < -DBL_MAX - rf) error(errfatal, errcodenumericunderflow, "Floating addition negative overflow!");

			v = (Value){ .Type = type_flt, .as.flt = lf + rf }; 
			break;
		}

		case type_sword: 
		{
			long ls = numsword(l);
			long rs = numsword(r);

			if (rs > 0 && ls > LONG_MAX - rs) error(errfatal, errcodenumericoverflow, "Signed addition overflow!");
			if (rs < 0 && ls < LONG_MIN - rs) error(errfatal, errcodenumericunderflow, "Signed addition underflow!");

			v = (Value){ .Type = type_sword, .as.sword = ls + rs }; 
			break;
		}

		default: 
		{
			unsigned long lw = numword(l);
			unsigned long rw = numword(r);

			if (lw > ULONG_MAX - rw) error(errfatal, errcodenumericoverflow, "Unsigned addition overflow!");

			v = (Value){ .Type = type_word, .as.word = lw + rw }; 
			break;
		}
	}

	return v;
}

static Value h_NumericSub 
(Value l, Value r)
{
	Value v;

	switch (numtype(l, r)) 
	{
		case type_flt: 
		{
			double lf = numflt(l);
			double rf = numflt(r);

			if (rf < 0.0 && lf > DBL_MAX + rf) error(errfatal, errcodenumericoverflow, "Floating subtraction positive overflow!");
			if (rf > 0.0 && lf < -DBL_MAX + rf) error(errfatal, errcodenumericunderflow, "Floating subtraction negative overflow!");

			v = (Value){ .Type = type_flt, .as.flt = lf - rf }; 
			break;	
		}

		case type_sword:
		{	
			long ls = numsword(l);
			long rs = numsword(r);

			if (rs > 0 && ls < LONG_MIN + rs) error(errfatal, errcodenumericunderflow, "Signed subtraction underflow!");
			if (rs < 0 && ls > LONG_MAX + rs) error(errfatal, errcodenumericoverflow, "Signed subtraction overflow!");

			v = (Value){ .Type = type_sword, .as.sword = ls - rs }; 
			break;
		}

		default: 
		{
			unsigned long lw = numword(l);
			unsigned long rw = numword(r);

			if (lw < rw) 
			/* XXX old behavior - error(errfatal, errcodenumericunderflow, "Unsigned subtraction underflow!"); */
			{
				if (lw > LONG_MAX || rw > LONG_MAX) error(errfatal, errcodenumericunderflow, "Unsigned subtraction underflow!");

				long ls = numsword(l);
				long rs = numsword(r);

				if (rs > 0 && ls < LONG_MIN + rs) error(errfatal, errcodenumericunderflow, "Signed subtraction underflow!");
				if (rs < 0 && ls > LONG_MAX + rs) error(errfatal, errcodenumericoverflow, "Signed subtraction overflow!");
			
				v = (Value){ .Type = type_sword, .as.sword = ls - rs };
				break;
			}

			v = (Value){ .Type = type_word, .as.word = lw - rw }; 
			break;
		}
	}

	return v;
}

static Value h_NumericMul 
(Value l, Value r)
{
	Value v;

	switch (numtype(l, r)) 
	{
		case type_flt: 
		{
			double lf = numflt(l);
			double rf = numflt(r);

			if (lf > 0.0)
			{
				if (rf > 0.0 && lf > DBL_MAX / rf) error(errfatal, errcodenumericoverflow, "Floating multiplcation overflow!");
				else if (rf < 0.0 && rf < -DBL_MAX / lf) error(errfatal, errcodenumericunderflow, "Floating multiplcation underflow!");
			} else if (lf < 0.0)
			{
				if (rf > 0.0 && lf < -DBL_MAX / rf) error(errfatal, errcodenumericunderflow, "Floating multiplcation underflow!");
				else if (rf < 0.0 && lf < DBL_MAX / rf) error(errfatal, errcodenumericoverflow, "Floating multiplcation overflow!");
			}

			v = (Value){ .Type = type_flt, .as.flt = numflt(l) * numflt(r) }; 
			break;
		}

		case type_sword: 
		{
			long ls = numsword(l);
			long rs = numsword(r);

			if (ls > 0)
			{
				if (rs > 0 && ls > LONG_MAX / rs) error(errfatal, errcodenumericoverflow, "Signed multiplcation overflow!");
				if (rs < 0 && rs < LONG_MIN / rs) error(errfatal, errcodenumericunderflow, "Signed multiplcation underflow!");
			} else if (ls < 0)
			{
				if (rs > 0 && ls < LONG_MAX / rs) error(errfatal, errcodenumericunderflow, "Signed multiplcation underflow!");
				else if (rs < 0 && ls < LONG_MAX / rs) error(errfatal, errcodenumericoverflow, "Signed multiplcation overflow!");
			}

			v = (Value){ .Type = type_sword, .as.sword = ls * rs }; 
			break;
		}

		default: 
		{
			unsigned long lw = numword(l);
			unsigned long rw = numword(r);

			if (rw != 0 && lw > ULONG_MAX / rw) error(errfatal, errcodenumericoverflow, "Unsigned multiplcation overflow!");

			v = (Value){ .Type = type_word, .as.word = lw * rw }; 
			break;
		}
	}

	return v;
}

static Value h_NumericDiv 
(Value l, Value r)
{
	Value v;

	switch (numtype(l, r)) 
	{
		case type_flt:
		{
			double lf = numflt(l);
			double rf = numflt(r);

			if (lf > 0.0)
			{
				if (rf > 0.0 && lf > DBL_MAX * rf) error(errfatal, errcodenumericoverflow, "Floating division overflow!");
				else if (rf < 0.0 && lf < -DBL_MAX * rf) error(errfatal, errcodenumericunderflow, "Floating division underflow!");
			} else if (lf < 0.0)
			{
				if (rf > 0.0 && lf < -DBL_MAX * rf) error(errfatal, errcodenumericunderflow, "Floating division underflow!");
				else if (rf < 0.0 && lf < DBL_MAX * rf) error(errfatal, errcodenumericoverflow, "Floating division overflow!");
			}

			if (rf == 0.0 || lf == 0.0) v = (Value){ .Type = type_word, .as.word = 0 };
			else v = (Value){ .Type = type_sword, .as.sword = (long)(lf / rf) };

			break;
		}
	
		case type_sword:
		{
			long ls = numsword(l);
			long rs = numsword(r);

			if (ls == LONG_MIN && rs == -1) error(errfatal, errcodenumericoverflow, "Signed division overflow!");

			if (rs == 0 || ls == 0) v = (Value){ .Type = type_word, .as.word = 0 };
			else v = (Value){ .Type = type_sword, .as.sword = ls / rs };

			break;
		}
	
		default:
		{
			unsigned long lw = numword(l);
			unsigned long rw = numword(r);

			if (rw == 0 || lw == 0) v = (Value){ .Type = type_word, .as.word = 0 };
			else v = (Value){ .Type = type_word, .as.word = lw / rw };

			break;
		}
	}

	return v;
}

static Value h_NumericMod 
(Value l, Value r)
{
	Value v;

	switch (numtype(l, r)) 
	{
		case type_flt:
		{
			double lf = numflt(l);
			double rf = numflt(r);

			if (rf == 0.0 || lf == 0.0) v = (Value){ .Type = type_word, .as.word = 0 };
			else v = (Value){ .Type = type_sword, .as.sword = (long)(lf / rf) };

			break;
		}
	
		case type_sword:
		{
			long ls = numsword(l);
			long rs = numsword(r);

			if (rs == 0 || ls == 0) v = (Value){ .Type = type_word, .as.word = 0 };
			else v = (Value){ .Type = type_sword, .as.sword = ls % rs };

			break;
		}
	
		default:
		{
			unsigned long lw = numword(l);
			unsigned long rw = numword(r);

			if (rw == 0 || lw == 0) v = (Value){ .Type = type_word, .as.word = 0 };
			else v = (Value){ .Type = type_word, .as.word = lw % rw };

			break;
		}
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
(Value v, char *buf, unsigned long n, struct Limits *limits)
{
	if (v.Type == type_str) return v.as.str;

	valuetostr(buf, n, v, limits);
	return buf;
}

static Value mkbool 
(int b)
{
	Value v;
	v.Type = type_word;
	v.as.word = b == 0;
	return v;
}

Value evalexprnode 
(ExprNode *node, VarMap *vars, Arena *persistAlloc, struct Limits *limits)
{
	Value l = evalexprdata(node->left, vars, persistAlloc, limits);
	Value r = evalexprdata(node->right, vars, persistAlloc, limits);
	char lb[64], rb[64];
	int cmp;

	switch (node->Op) 
	{
		case ExprOp_NumericAdd: return h_NumericAdd(l, r);
		case ExprOp_NumericSub: return h_NumericSub(l, r);
		case ExprOp_NumericMul: return h_NumericMul(l, r);
		case ExprOp_NumericDiv: return h_NumericDiv(l, r);
		case ExprOp_NumericMod: return h_NumericMod(l, r);

		case ExprOp_NumericGreaterThan:
			cmp = h_NumericCmp(l, r);
			return mkbool(cmp > 0);

		case ExprOp_NumericLessThan:
			cmp = h_NumericCmp(l, r);
			return mkbool(cmp < 0);

		case ExprOp_NumericGreaterThanOrEqualTo:
			cmp = h_NumericCmp(l, r);
			return mkbool(cmp >= 0);
	
		case ExprOp_NumericLessThanOrEqualTo:
			cmp = h_NumericCmp(l, r);
			return mkbool(cmp <= 0);

		return mkbool(cmp >= 0);

		case ExprOp_NumericEqualTo:
			cmp = h_NumericCmp(l, r);
			return mkbool(cmp == 0);

		case ExprOp_NumericNotEqualTo:
			cmp = h_NumericCmp(l, r);
			return mkbool(cmp != 0);

		case ExprOp_StringEqualTo:
			return mkbool(cmpstr(asstr(l, lb, sizeof(lb), limits), asstr(r, rb, sizeof(rb), limits)) == 0);

		case ExprOp_StringContains:
		case ExprOp_StringStartsWith:
		case ExprOp_StringEndsWith: 
		{
			const char *ls = asstr(l, lb, sizeof(lb), limits);
			const char *rs = asstr(r, rb, sizeof(rb), limits);
			unsigned long ll = getstrlen(ls);
			unsigned long rl = getstrlen(rs);

			if (rl > ll) return mkbool(0);

			unsigned long start = 0;
			unsigned long end = ll - rl;

			if (node->Op == ExprOp_StringStartsWith) end = 0;
			if (node->Op == ExprOp_StringEndsWith) start = end;

			for (unsigned long i = start; i <= end; i++)
				if (cmpstrn(ls + i, rs, rl) == 0) return mkbool(1);

			return mkbool(0);
		}

		case ExprOp_StringConcat: 
		{
			const char *ls = asstr(l, lb, sizeof(lb), limits);
			const char *rs = asstr(r, rb, sizeof(rb), limits);
			unsigned long ll = getnstrlen(ls);
			unsigned long rl = getnstrlen(rs);

			if ((ll + rl + 1) >= limits->misc_strlen) error(errfatal, errcodelimitreached, "String is too long!");
			char *out = alloc(persistAlloc, ll + rl + 1);

			copymem(ls, out, ll);
			copymem(rs, out + ll, rl);
			out[ll + rl] = '\0';

			Value v;
			v.Type = type_str;
			v.as.str = out;
			return v;
		}

		case ExprOp_StringFind:
		{
			const char *ls = asstr(l, lb, sizeof(lb), limits);
			const char *rs = asstr(r, rb, sizeof(rb), limits);

			unsigned long pos = 0;
			char *str, *ostr = NULL;

			Value *vstr = getVar(vars, ls);
			Value *vstr2 = getVar(vars, rs);

			if (vstr == NULL) str = ls;
			else 
			{
				char *strbuff = alloc(persistAlloc, limits->misc_strlen);
				asstr(*vstr, strbuff, limits->misc_strlen, limits);
				str = strbuff;
			}

			if (vstr2 == NULL) ostr = ls;
			else 
			{
				char *strbuff2 = alloc(persistAlloc, limits->misc_strlen);
				asstr(*vstr2, strbuff2, limits->misc_strlen, limits);
				ostr = strbuff2;
			}

			char *index = strtokn(str, ostr);
			if (index != NULL) pos = (unsigned long)(index - ls) + 1;

			Value v;
			v.Type = type_word;
			v.as.word = pos;
			return v;
		}

		case ExprOp_StringSplitRight: 
		{
			const char *ls = asstr(l, lb, sizeof(lb), limits);
			const char *rs = asstr(r, rb, sizeof(rb), limits);

			char *str, *ostr = NULL;
			unsigned long pos = 0;

			Value *vstr = getVar(vars, ls);
			Value *vstr2 = getVar(vars, rs);

			if (vstr == NULL) str = ls;
			else 
			{
				char *strbuff = alloc(persistAlloc, limits->misc_strlen);
				asstr(*vstr, strbuff, limits->misc_strlen, limits);
				str = strbuff;
			}

			if (vstr2 == NULL) pos = numword(r);
			else pos = numword(*vstr2);

			unsigned long ostrlen = (getstrlen(str) - pos);
			ostr = alloc(persistAlloc, ostrlen);
			char *pstr = str + pos;

			if (pos > ostrlen) pos = ostrlen;

			ostr[ostrlen] = '\0';
			copymem(pstr, ostr, ostrlen);

			return (Value){ .Type = type_str, .as.str = ostr };
		}

		case ExprOp_StringSplitLeft: 
		{
			const char *ls = asstr(l, lb, sizeof(lb), limits);
			const char *rs = asstr(r, rb, sizeof(rb), limits);

			char *str, *ostr = NULL;
			unsigned long pos = 0;

			Value *vstr = getVar(vars, ls);
			Value *vstr2 = getVar(vars, rs);

			if (vstr == NULL) str = ls;
			else 
			{
				char *strbuff = alloc(persistAlloc, limits->misc_strlen);
				asstr(*vstr, strbuff, limits->misc_strlen, limits);
				str = strbuff;
			}

			if (vstr2 == NULL) pos = numword(r);
			else pos = numword(*vstr2);

			unsigned long strlen = getstrlen(str);
			if (pos > strlen) pos = strlen;
			unsigned long ostrlen = pos - 1;
			ostr = alloc(persistAlloc, ostrlen);
			char *pstr = str + pos;

			copymem(str, ostr, ostrlen);
			ostr[ostrlen] = '\0';

			return (Value){ .Type = type_str, .as.str = ostr };
		}
	}

	return mkbool(0);
	/* XXX the inversion is not a mistake */
}

Value evalexprdata 
(ExprNodeData data, VarMap *vars, Arena *persistAlloc, struct Limits *limits)
{
	if (data.Type == ExprDataVal) return resolveleaf(data.value, vars);
	return evalexprnode(data.Node, vars, persistAlloc, limits);
}

Value evalstr 
(const char *str, int *ok, VarMap *vars, Arena *tempAlloc, Arena *persistAlloc, struct Limits *limits)
{ return evalexprdata(str2expr(str, ok, tempAlloc, persistAlloc), vars, persistAlloc, limits); }
