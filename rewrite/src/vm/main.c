#include <stdio.h>
#include <stdlib.h>

#include "../allocator/allocator.h"
#include "../state/variables.h"
#include "../state/functions.h"

#include "../platform/use/print.h"
#include "../platform/use/getstrlen.h"
#include "../platform/use/copystr.h"

int main() {
	struct Arena *arena = arena = initAlloc(4096);
	Map vars = initVars(32);

	char *msg1 = "Arena allocation working (1)\n";
	char *msg2 = "Arena allocation working (2)\n";
	char *msg3 = "Variable tables working (1)\n";
	char *msg4 = "Variable tables working (2)\n";

	char *area1 = alloc(arena, getstrlen(msg1));
	char *area2 = alloc(arena, getstrlen(msg2));
	char *area3 = alloc(arena, getstrlen(msg3));
	char *area4 = alloc(arena, getstrlen(msg4));

	copystr(msg1, area1);
	copystr(msg2, area2);
	copystr(msg3, area3);
	copystr(msg4, area4);

	print(area1);
	print(area2);

	putVar(&vars, "v1", area3);
	putVar(&vars, "v2", area4);

	char *result1 = getVar(&vars, "v1");
	char *result2 = getVar(&vars, "v2");

	print(result1);
	print(result2);

	return 0;
}
