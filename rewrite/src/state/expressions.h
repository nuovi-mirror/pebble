#pragma once

#include "values.h"
#include "../platform/use/cmpstr.h"
#include "../platform/use/skipspace.h"
#include "../platform/use/lalloc.h"

typedef struct ExprNode ExprNode;

typedef enum 
{
	ExprOp_NumericAdd,
	ExprOp_NumericSub,
	ExprOp_NumericMul,
	ExprOp_NumericDiv,
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
	ExprNodeData left;
	ExprNodeData right;
};

typedef struct
{
	const char *Sym;
	ExprOperation Op;
} ExprOperator;

const 
ExprOperator ExprOperators[] = 
{
	{ "+", ExprOp_NumericAdd },
	{ "-", ExprOp_NumericSub },
	{ "*", ExprOp_NumericMul },
	{ "/", ExprOp_NumericDiv },
};

ExprOperator *strtooperator
(const char *str)
{
	for (unsigned long i = 0; i < sizeof(ExprOperators) 
			/ sizeof(ExprOperators[1]); i++)
	{
		if (cmpstr(ExprOperators[i].Sym, str) == 0)
		{
			return &ExprOperators[i];
			break;
		}
	}

	return NULL;
}
