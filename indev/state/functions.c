#include "functions.h"
#include "hashmap.h"

unsigned long hasFunc
(FuncMap *m, const char *str)		
{ return mapHash(m, str); }

FuncMap initFuncs(unsigned long cap)			
{ return initHashMap(cap); }

void putFunc
(FuncMap *m, const char *key, const void *value)	
{ hashMapPut(m, key, value); }

void *getFunc
(FuncMap *m, const char *key)		
{ return hashMapGet(m, key); }

void freeFuncs
(FuncMap *m)				
{ hashMapFreeMap(m); }
