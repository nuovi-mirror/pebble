#ifndef h_variables_
#define h_variables_
#include "allocator.h"
#include "hashmap.h"

typedef struct VarMap {
	SHashMap *map;
	unsigned long max;
	unsigned long arrcount;
	unsigned long mapcount;
	void **arr;
} VarMap;

/* XXX unused
static unsigned long hashVar
(VarMap *m, const char *str);
*/

VarMap initVars (unsigned long cap, Arena *persistAlloc);
void putVar (VarMap *m, const char *key, const void *value, Arena *persistAlloc);
void *getVar (VarMap *m, const char *key);
void freeVars (VarMap *m);
#endif
