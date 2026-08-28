#pragma once

#include "values.h"

typedef struct
{
	Value Data; /* data in the node */
	ExpressionNode *Left; /* node to the left of this one */
	ExpressionNode *Right; /* node to the right of this one */
} ExpressionNode;
