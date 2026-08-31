#ifndef STATE_EXPRESSIONS_H_
#define STATE_EXPRESSIONS_H_

#include "values.h"
#include "../platform/use/cmpstr.h"
#include "../platform/use/skipspace.h"
#include "../platform/use/lalloc.h"
#include "../platform/use/cmpstr.h"
#include "../platform/use/cmpstrn.h"
#include "../platform/use/copymem.h"
#include "../platform/use/snprint.h"

typedef struct ExprNode ExprNode;

typedef enum 
{
	ExprOp_NumericAdd,
	ExprOp_NumericSub,
	ExprOp_NumericMul,
	ExprOp_NumericDiv,
	ExprOp_NumericGreaterThan,
	ExprOp_NumericLessThan,
	ExprOp_NumericEqualTo,
	ExprOp_NumericNotEqualTo,
	ExprOp_StringEqualTo,
	ExprOp_StringContains,
	ExprOp_StringEndsWith,
	ExprOp_StringStartsWith,
	ExprOp_StringConcat,
} ExprOperation;

typedef enum 
{
	ExprDataVal,
	ExprDataNode,
} ExprNodeDataType;

typedef struct 
{
	ExprNodeDataType Type;
	union 
	{
		Value value;
		ExprNode *Node;
	};
} ExprNodeData;

struct ExprNode
{
	ExprOperation Op;
	char *Source;
	ExprNodeData left;
	ExprNodeData right;
};

typedef struct
{
	const char *Sym;
	ExprOperation Op;
	int Pres; /* eval order - lower = sooner */
} ExprOperator;

const 
ExprOperator ExprOperators[] = 
{
	{ "+", 		ExprOp_NumericAdd, 		1 },
	{ "-", 		ExprOp_NumericSub, 		1 },
	{ "*", 		ExprOp_NumericMul, 		0 },
	{ "/", 		ExprOp_NumericDiv, 		0 },
	{ ">", 		ExprOp_NumericGreaterThan, 	2 },
	{ "<", 		ExprOp_NumericLessThan, 	2 },
	{ "==", 	ExprOp_NumericEqualTo, 		2 },
	{ "!=", 	ExprOp_NumericNotEqualTo, 	2 },
	{ "?=", 	ExprOp_StringEqualTo, 		4 },
	{ "-?=", 	ExprOp_StringContains, 		4 },
	{ "e?=", 	ExprOp_StringEndsWith, 		4 },
	{ "s?=", 	ExprOp_StringStartsWith, 	4 },
	{ "s++", 	ExprOp_StringConcat, 		3 },
};

typedef enum
{
	Token_End,
	Token_Value,
	Token_Operator,
	Token_LeftParent,
	Token_RightParent,
} TokenType;

typedef struct 
{
	TokenType Type;
	const ExprOperator *Op; /* set if Type == Token_Operator */
	Value Value; /* set if Type = Token_Value */
} Token;

typedef struct 
{
	const char *cursor;
	Token lookahead;
	int error; /* = 1 if parser error */
} ExprParser;

ExprOperator *strtooperator
(const char *str)
{
	for (unsigned long i = 0; i < sizeof(ExprOperators) / sizeof(ExprOperators[1]); i++)
	{
		if (cmpstr(ExprOperators[i].Sym, str) == 0)
		{
			return &ExprOperators[i];
			break;
		}
	}

	return NULL;
}

Value parseliteral
(const char **str)
{
	const char *start = *str;
	const char *p = start;

	while (
			*p != '\0' && 
			*p != ' ' && 
			*p != '\t' &&
			*p != '\n' &&
			*p !=  '(' &&
			*p != ')')
		p++;

	unsigned long len = (unsigned long)(p - start);
	char *buf = lalloc(len);
	copymem(buf, start, len);
	buf[len] = '\0';

	*str = p;

	return guessvaluetype(buf);
}

void nexttoken
(const char **str, Token *token)
{
	*str = tskipspace(*str);

	if (**str == '\0')
	{
		token->Type = Token_End;
		return;
	}

	if (**str == '(')
	{
		(*str)++;
		token->Type = Token_LeftParent;
		return;
	}

	if (**str == ')')
	{
		(*str)++;
		token->Type = Token_RightParent;
		return;
	}

	/* try matching an operator symbol */
	const ExprOperator *best = NULL;
	unsigned long bestlen = 0;

	for (unsigned long i = 0; i < sizeof(ExprOperators) / sizeof(ExprOperators[0]); i++)
	{
		unsigned long len = strlen(ExprOperators[i].Sym);
		if (cmpstrn(*str, ExprOperators[i].Sym, len) == 0)
		{
			best = &ExprOperators[i];
			bestlen = len;
		}
	}

	if (best)
	{
		*str += bestlen;
		token->Type = Token_Operator;
		token->Op = best;
		return;
	}

	/* value literal from here */
	token->Type = Token_Value;
	token->Value = parseliteral(str);
}

int exprnodetostr
(char *buf, unsigned long bufsize, ExprNode *node)
{
	if (node->Source == NULL)
		return -1;

	return snprint(buf, bufsize, "%s", node->Source);
}

static void parseradvance
(ExprParser *p)
{
	nexttoken(&p->cursor, &p->lookahead);
}

ExprNode *newexprnode
(ExprOperation op, ExprNodeData left, ExprNodeData right)
{
	ExprNode *n = lalloc(sizeof(ExprNode));
	
	n->Op = op;
	n->left = left;
	n->right = right;

	return n;
}

ExprNodeData parseexpr(ExprParser *p, int maxPrec);

ExprNodeData parseprimary
(ExprParser *p)
{
	ExprNodeData data;

	if (p->lookahead.Type == Token_LeftParent)
	{
		parseradvance(p); /* consume '(' */
		/* set that to whatever the currest loosest tier is */
		ExprNodeData inner = parseexpr(p, 2);

		if (p->lookahead.Type != Token_RightParent)
			p->error = 1;
		else
			parseradvance(p); /* consume ')' */
		return inner;
	}

	if (p->lookahead.Type == Token_Value)
	{
		data.Type = ExprDataVal;
		data.value = p->lookahead.Value;
		parseradvance(p);
		return data;
	}

	/* malformed input */
	p->error = 1;
	data.Type = ExprDataVal;
	data.value.Type = type_word;
	data.value.as.word = 0;
	return data;
}

ExprNodeData parseexpr
(ExprParser *p, int maxPrec)
{
	ExprNodeData left = parseprimary(p);

	while (
			!p->error && 
			p->lookahead.Type == Token_Operator &&
			p->lookahead.Op->Pres <= maxPrec)
	{
		const ExprOperator *op = p->lookahead.Op;
		parseradvance(p); /* consume operator */

		/* op->Pres - 1: left-associative */
		/* next call may only take operators
		 * strictly tigher than this one */
		ExprNodeData right = parseexpr(p, op->Pres - 1);

		ExprNode *node = newexprnode(op->Op, left, right);

		ExprNodeData wrapped;
		wrapped.Type = ExprDataNode;
		wrapped.Node = node;
		left = wrapped;
	}

	return left;
}

/* entry point */
ExprNodeData str2expr
(const char *str, int *ok)
{
	ExprParser p;
	p.cursor = str;
	p.error = 0;
	parseradvance(&p); /* prime lookahead */

	int maxLevel = 4;
	for (unsigned long i = 0; i < sizeof(ExprOperators) / sizeof(ExprOperators[0]); i++)
		if (ExprOperators[i].Pres > maxLevel)
			maxLevel = ExprOperators[i].Pres;

	ExprNodeData result = parseexpr(&p, maxLevel);

	if (p.lookahead.Type != Token_End)
		p.error = 1; /* trailing garbage */

	if (!p.error && result.Type == ExprDataNode)
	{
		unsigned long len = getstrlen(str);
		char *copy = lalloc(len + 1);
		copymem(copy, str, len + 1);
		result.Node->Source = copy;
	}

	if (ok != NULL)
		*ok = !p.error;

	return result;
}

/* type guesser infa */
int isexpression
(const char *str)
{
	const char *p = str;

	while (*p != '\0')
	{
		const ExprOperator *best = NULL;
		unsigned long bestlen = 0;

		/* longest match at this position, same rule nexttoken uses */
		for (unsigned long i = 0; i < sizeof(ExprOperators) / sizeof(ExprOperators[0]); i++)
		{
			unsigned long len = strlen(ExprOperators[i].Sym);
			if (cmpstrn(p, ExprOperators[i].Sym, len) == 0 && len > bestlen)
			{
				best = &ExprOperators[i];
				bestlen = len;
			}
		}

		if (best)
		{
			unsigned long i = (unsigned long)(p - str);

			/* need exactly one space to the left: str[i-1]==' ' and str[i-2]!=' ' */
			int leftok = (i >= 2) && (str[i-1] == ' ') && (str[i-2] != ' ');

			/* need exactly one space to the right, followed by a real char */
			const char *after = p + bestlen;
			int rightok = (after[0] == ' ') && (after[1] != '\0') && (after[1] != ' ');

			if (leftok && rightok)
				return 1;

			/* not a valid flanked operator here - keep scanning past it */
			p += bestlen;
			continue;
		}

		p++;
	}

	return 0;
}

/* wrapper */
Value guessvaluetypeorexpr
(char *data)
{
	if (isexpression(data))
	{
		Value v;
		int ok;
		ExprNodeData tree = str2expr(data, &ok);

		v.Type = type_expr;
		v.as.expr = (ok && tree.Type == ExprDataNode) ? tree.Node : NULL;

		/*
		if (v.as.expr == NULL)
			v.Type = type_null;
		*/

		return v;
	}

	return guessvaluetype(data);
}

#endif
