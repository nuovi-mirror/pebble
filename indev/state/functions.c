#include "functions.h"
#include "hashmap.h"
#include "allocator.h"

unsigned long hasFunc
(FuncMap *m, const char *str)		
{ return mapHash(m, str); }

FuncMap initFuncs(unsigned long cap)			
{ return initHashMap(cap); }

void putFunc
(FuncMap *m, const char *key, const void *value, Arena *persistAlloc)	
{ hashMapPut(m, key, value, persistAlloc); }

void *getFunc
(FuncMap *m, const char *key)		
{ return hashMapGet(m, key); }

void freeFuncs
(FuncMap *m)				
{ hashMapFreeMap(m); }
