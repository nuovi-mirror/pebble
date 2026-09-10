#include "variables.h"
#include "hashmap.h"

unsigned long hashVar(VarMap *m, char *str)		{ return mapHash(m, str); }
VarMap initVars(unsigned long cap)			{ return initHashMap(cap); }
void   putVar(VarMap *m, char *key, void *value)		{ hashMapPut(m, key, value); }
void  *getVar(VarMap *m, char *key)			{ return hashMapGet(m, key); }
void   freeVars(VarMap *m)				{ hashMapFreeMap(m); }
