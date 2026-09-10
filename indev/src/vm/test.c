#include <stdio.h>
#include <stdlib.h>

#include "../allocator/allocator.h"

#include "../state/variables.h"
#include "../state/functions.h"
#include "../state/values.h"

#include "../platform/use/print.h"
#include "../platform/use/getstrlen.h"
#include "../platform/use/copystr.h"

void testAllocator(void) {
	struct Arena *arena = arena = initAlloc(4096);

	char *arenamsg1 = "Arena allocation working (1)\n";
	char *arenamsg2 = "Arena allocation working (2)\n";

	char *arenaarea1 = alloc(arena, getstrlen(arenamsg1));
	char *arenaarea2 = alloc(arena, getstrlen(arenamsg2));

	copystr(arenamsg1, arenaarea1);
	copystr(arenamsg2, arenaarea2);

	print(arenaarea1);
	print(arenaarea2);

	freeAllocator(arena);
}

void testVariables(void) {
	struct Arena *arena = arena = initAlloc(4096);
	VarMap vars = initVars(32);
	
	char *varsmsg1 = "Variable tables working (1)\n";
	char *varsmsg2 = "Variable tables working (2)\n";

	char *varsarea1 = alloc(arena, getstrlen(varsmsg1));
	char *varsarea2 = alloc(arena, getstrlen(varsmsg2));

	copystr(varsmsg1, varsarea1);
	copystr(varsmsg2, varsarea2);

	putVar(&vars, "one", varsarea1);
	putVar(&vars, "two", varsarea2);

	char *varresult1 = getVar(&vars, "one");
	char *varresult2 = getVar(&vars, "two");

	print(varresult1);
	print(varresult2);

	freeAllocator(arena);
}

int main(void) {
	testAllocator();
	testVariables();

	return 0;
}
