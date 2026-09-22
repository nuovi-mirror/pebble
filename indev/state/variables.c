#include "variables.h"
#include "hashmap.h"
#include "allocator.h"

unsigned long hashVar
(VarMap *m, const char *str)		
{ return mapHash(m, str); }

VarMap initVars
(unsigned long cap, Arena *persistAlloc)			
{ return initHashMap(cap, persistAlloc); }

void putVar
(VarMap *m, const char *key, const void *value, Arena *persistAlloc)		
{ hashMapPut(m, key, value, persistAlloc); }

void *getVar
(VarMap *m, const char *key)			
{ return hashMapGet(m, key); }

void freeVars
(VarMap *m)				
{ hashMapFreeMap(m); }
