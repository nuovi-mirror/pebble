#include "evaluator.h"
#include "cmpstr.h"
#include "cmpstrn.h"
#include "copymem.h"
#include "expressions.h"
#include "getnstrlen.h"
#include "getstrlen.h"
#include "str2d.h"
#include "str2ul.h"
#include "str2l.h"
#include "main.h"
#include "values.h"
#include "variables.h"
#include "escape.h"
#include "unescape.h"

#include <limits.h>
#include <math.h>
#include <float.h>

#include "print.h" /* XXX debug */

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

static Value h_NumericAdd 
(Value l, Value r)
{
	Value v;

	switch (numtype(l, r)) 
	{
		case type_flt: 
		{
			flt lf = numflt(l);
			flt rf = numflt(r);

			if (rf > 0.0 && lf > FLT_MAX - rf) error(errfatal, errcodenumericoverflow, "Floating addition positive overflow!");
			if (rf < 0.0 && lf < -FLT_MAX - rf) error(errfatal, errcodenumericunderflow, "Floating addition negative overflow!");

			v = (Value){ .Type = type_flt, .as.flt = lf + rf }; 
			break;
		}

		case type_sword: 
		{
			sword ls = numsword(l);
			sword rs = numsword(r);

			if (rs > 0 && ls > SWORD_MAX - rs) error(errfatal, errcodenumericoverflow, "Signed addition overflow!");
			if (rs < 0 && ls < SWORD_MIN - rs) error(errfatal, errcodenumericunderflow, "Signed addition underflow!");

			v = (Value){ .Type = type_sword, .as.sword = ls + rs }; 
			break;
		}

		default: 
		{
			word lw = numword(l);
			word rw = numword(r);

			if (lw > WORD_MAX - rw) error(errfatal, errcodenumericoverflow, "Unsigned addition overflow!");

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
			flt lf = numflt(l);
			flt rf = numflt(r);

			if (rf < 0.0 && lf > FLT_MAX + rf) error(errfatal, errcodenumericoverflow, "Floating subtraction positive overflow!");
			if (rf > 0.0 && lf < -FLT_MAX + rf) error(errfatal, errcodenumericunderflow, "Floating subtraction negative overflow!");

			v = (Value){ .Type = type_flt, .as.flt = lf - rf }; 
			break;	
		}

		case type_sword:
		{	
			sword ls = numsword(l);
			sword rs = numsword(r);

			if (rs > 0 && ls < SWORD_MIN + rs) error(errfatal, errcodenumericunderflow, "Signed subtraction underflow!");
			if (rs < 0 && ls > SWORD_MAX + rs) error(errfatal, errcodenumericoverflow, "Signed subtraction overflow!");

			v = (Value){ .Type = type_sword, .as.sword = ls - rs }; 
			break;
		}

		default: 
		{
			word lw = numword(l);
			word rw = numword(r);

			if (lw < rw) 
			/* XXX old behavior - error(errfatal, errcodenumericunderflow, "Unsigned subtraction underflow!"); */
			{
				if (lw > SWORD_MAX || rw > SWORD_MAX) error(errfatal, errcodenumericunderflow, "Unsigned subtraction underflow!");

				sword ls = numsword(l);
				sword rs = numsword(r);

				if (rs > 0 && ls < SWORD_MIN + rs) error(errfatal, errcodenumericunderflow, "Signed subtraction underflow!");
				if (rs < 0 && ls > SWORD_MAX + rs) error(errfatal, errcodenumericoverflow, "Signed subtraction overflow!");
			
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
			flt lf = numflt(l);
			flt rf = numflt(r);

			if (lf > 0.0)
			{
				if (rf > 0.0 && lf > FLT_MAX / rf) error(errfatal, errcodenumericoverflow, "Floating multiplcation overflow!");
				else if (rf < 0.0 && rf < -FLT_MAX / lf) error(errfatal, errcodenumericunderflow, "Floating multiplcation underflow!");
			} else if (lf < 0.0)
			{
				if (rf > 0.0 && lf < -FLT_MAX / rf) error(errfatal, errcodenumericunderflow, "Floating multiplcation underflow!");
				else if (rf < 0.0 && lf < FLT_MAX / rf) error(errfatal, errcodenumericoverflow, "Floating multiplcation overflow!");
			}

			v = (Value){ .Type = type_flt, .as.flt = numflt(l) * numflt(r) }; 
			break;
		}

		case type_sword: 
		{
			sword ls = numsword(l);
			sword rs = numsword(r);

			if (ls > 0)
			{
				if (rs > 0 && ls > SWORD_MAX / rs) error(errfatal, errcodenumericoverflow, "Signed multiplcation overflow!");
				if (rs < 0 && rs < SWORD_MIN / ls) error(errfatal, errcodenumericunderflow, "Signed multiplcation underflow!");
			} else if (ls < 0)
			{
				if (rs > 0 && ls < SWORD_MIN / rs) error(errfatal, errcodenumericunderflow, "Signed multiplcation underflow!");
				else if (rs < 0 && ls < SWORD_MAX / rs) error(errfatal, errcodenumericoverflow, "Signed multiplcation overflow!");
			}

			v = (Value){ .Type = type_sword, .as.sword = ls * rs }; 
			break;
		}

		default: 
		{
			word lw = numword(l);
			word rw = numword(r);

			if (rw != 0 && lw > WORD_MAX / rw) error(errfatal, errcodenumericoverflow, "Unsigned multiplcation overflow!");

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
			flt lf = numflt(l);
			flt rf = numflt(r);

			if (rf == 0.0 || lf == 0.0) v = (Value){ .Type = type_word, .as.word = 0 };
			else
			{
				flt alf = lf < 0.0 ? -lf : lf;
				flt arf = rf < 0.0 ? -rf : rf;

				if (arf < 1.0 && alf > FLT_MAX * arf)
				{
					if ((lf < 0.0) == (rf < 0.0)) error(errfatal, errcodenumericoverflow, "Floating dividion overflow!");
					else error(errfatal, errcodenumericunderflow, "Floating division underflow!");
				}

				flt q = lf / rf;

				if (!(q >= (flt)SWORD_MIN)) error(errfatal, errcodenumericunderflow, "Floating division underflow!");
				if (!(q < -(flt)SWORD_MIN)) error(errfatal, errcodenumericoverflow, "Floating division overflow!");

				v = (Value){ .Type = type_sword, .as.sword = (sword)q };
			}

			break;
		}
	
		case type_sword:
		{
			sword ls = numsword(l);
			sword rs = numsword(r);

			if (rs == 0 || ls == 0) v = (Value){ .Type = type_word, .as.word = 0 };
			else
			{
				if (ls == SWORD_MIN && rs == -1) error(errfatal, errcodenumericoverflow, "Signed division overflow!");
				v = (Value){ .Type = type_sword, .as.sword = ls / rs };
			}

			break;
		}
	
		default:
		{
			word lw = numword(l);
			word rw = numword(r);

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
			flt lf = numflt(l);
			flt rf = numflt(r);

			if (rf == 0.0 || lf == 0.0) v = (Value){ .Type = type_word, .as.word = 0 };
			else 
			{
				flt m = lf < 0.0 ? -lf : lf;
				flt d = rf < 0.0 ? -rf : rf;

				if (m >= d)
				{
					flt t = d;
					while (t <= FLT_MAX / 2 && t + t <= m && t + t > t) t += t;
					for (;;)
					{
						if (m >= t) m -= t;
						if (t <= d) break;
						t *= 0.5;
					}
				}

				if (lf < 0.0) m = -m;

				if (!(m >= (flt)SWORD_MIN)) error(errfatal, errcodenumericunderflow, "Floating modulo underflow!");
				if (!(m < -(flt)SWORD_MIN)) error(errfatal, errcodenumericoverflow, "Floating modulo overflow!");

				v = (Value){ .Type = type_sword, .as.sword = (sword)m };
			}

			break;
		}
	
		case type_sword:
		{
			sword ls = numsword(l);
			sword rs = numsword(r);

			if (rs == 0 || ls == 0) v = (Value){ .Type = type_word, .as.word = 0 };
			else
			{
				if (ls == SWORD_MIN && rs == -1) error(errfatal, errcodenumericoverflow, "Signed modulo overflow!");
				v = (Value){ .Type = type_sword, .as.sword = ls % rs };
			}

			break;
		}
	
		default:
		{
			word lw = numword(l);
			word rw = numword(r);

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
		case type_flt: 
		{
			flt lf = numflt(l);
			flt rf = numflt(r);
			return (lf > rf) - (lf < rf);
		}

		case type_sword: 
		{
			sword ls = numsword(l);
			sword rs = numsword(r);
			return (ls > rs) - (ls < rs);
		}

		default:
		{
			word lw = numword(l);
			word rw = numword(r);
			return (lw > rw) - (lw < rw);
		}
	}
}

static const char *asstr 
(Value v, char *buf, word n, struct Limits *limits)
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

static Value mkstr
(const char *src, word len, Arena *persistAlloc)
{
	char *out = alloc(persistAlloc, len + 1);

	copymem(src, out, len);
	out[len] = '\0';

	return (Value){ .Type = type_str, .as.str = out };
}

static word findsub
(const char *hay, const char *needle)
{
	word hl = getstrlen(hay);
	word nl = getstrlen(needle);

	if (nl == 0 || nl > hl) return 0;

	for (word i = 0; i + nl <= hl; i++)
		if (cmpstrn(hay + i, needle, nl) == 0) return i + 1;

	return 0;
}

Value evalexprnode 
(ExprNode *node, VarMap *vars, Arena *persistAlloc, struct Limits *limits)
{
	Value l = evalexprdata(node->left, vars, persistAlloc, limits);
	Value r = evalexprdata(node->right, vars, persistAlloc, limits);
	char lb[128], rb[128];
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
			word ll = getstrlen(ls);
			word rl = getstrlen(rs);

			if (rl > ll) return mkbool(0);

			word start = 0;
			word end = ll - rl;

			if (node->Op == ExprOp_StringStartsWith) end = 0;
			if (node->Op == ExprOp_StringEndsWith) start = end;

			for (word i = start; i <= end; i++)
				if (cmpstrn(ls + i, rs, rl) == 0) return mkbool(1);

			return mkbool(0);
		}

		case ExprOp_StringConcat: 
		{
			const char *ls = asstr(l, lb, sizeof(lb), limits);
			const char *rs = asstr(r, rb, sizeof(rb), limits);
			word ll = getnstrlen(ls);
			word rl = getnstrlen(rs);

			if ((ll + rl + 1) >= limits->misc_strlen) error(errfatal, errcodelimitreached, "String is too sword!");
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
			const char *hay = asstr(l, lb, sizeof(lb), limits);
			const char *needle = asstr(r, rb, sizeof(rb), limits);

			return (Value){ .Type = type_word, .as.word = findsub(hay, needle) };
		}

		case ExprOp_StringSplitLeft:
		{
			const char *str = asstr(l, lb, sizeof(lb), limits);
			word len = getstrlen(str);
			word pos = numword(r);

			if (pos == 0 || pos > len) return mkstr(str, len, persistAlloc);
			return mkstr(str, pos - 1, persistAlloc);
		}

		case ExprOp_StringSplitRight:
		{
			const char *str = asstr(l, lb, sizeof(lb), limits);
			word len = getstrlen(str);
			word pos = numword(r);

			if (pos > len) return mkstr(str, len, persistAlloc);
			return mkstr(str + pos, len - pos, persistAlloc);
		}

		case ExprOp_StringLength:
		{
			const char *str = asstr(l, lb, sizeof(lb), limits);
			return (Value){ .Type = type_word, .as.word = getstrlen(str) };
		}

		case ExprOp_StringEscape:
		{
			const char *str = asstr(l, lb, sizeof(lb), limits);
			char pb[sizeof(lb)];
			escape(str, pb, sizeof(pb));
			word pblen = getstrlen(pb);
			return mkstr(pb, pblen, persistAlloc);
		}

		case ExprOp_StringUnEscape:
		{
			const char *str = asstr(l, lb, sizeof(lb), limits);
			char pb[sizeof(lb)];
			unescape(str, pb, sizeof(pb));
			word pblen = getstrlen(pb);
			return mkstr(pb, pblen, persistAlloc);
		}
	}

	return mkbool(0);
	/* XXX the inversion is not a mistake */
}

Value evalexprdata 
(ExprNodeData data, VarMap *vars, Arena *persistAlloc, struct Limits *limits)
{
	if (data.Type == ExprDataVal) return resolveleaf(data.data.value, vars);
	return evalexprnode(data.data.Node, vars, persistAlloc, limits);
}

Value evalstr 
(const char *str, int *ok, VarMap *vars, Arena *tempAlloc, Arena *persistAlloc, struct Limits *limits)
{ return evalexprdata(str2expr(str, ok, tempAlloc, persistAlloc), vars, persistAlloc, limits); }
