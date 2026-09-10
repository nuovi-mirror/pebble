#ifndef STATE_EXPRESSIONS_H_
#define STATE_EXPRESSIONS_H_

#include "values.h"
#include "allocator.h"

/* typedef struct ExprNode ExprNode; */
/* we get this from values.h */

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

/* moved from values.h */
int valuetostr(char *buff, unsigned long buffsize, Value v);

typedef struct
{
	const char *Sym;
	ExprOperation Op;
	int Pres; /* eval order - lower = sooner */
} ExprOperator;

extern const 
ExprOperator ExprOperators[];

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

const ExprOperator *strtooperator
(const char *str);

Value parseliteral
(const char **str, Arena *arena);

void nexttoken
(const char **str, Token *token, Arena *arena);

int exprnodetostr
(char *buf, unsigned long bufsize, ExprNode *node);

ExprNode *newexprnode
(ExprOperation op, ExprNodeData left, ExprNodeData right, Arena *arena);

ExprNodeData parseexpr
(ExprParser *p, int maxPrec, Arena *arena);

ExprNodeData parseprimary
(ExprParser *p, Arena *arena);

/* entry point */
ExprNodeData str2expr
(const char *str, int *ok, Arena *arena);

/* type guesser infa */
int isexpression
(const char *str);

/* wrapper */
Value guessvaluetypeorexpr
(char *data, Arena *arena);

#endif
