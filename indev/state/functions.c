#include "functions.h"
#include "hashmap.h"

typedef SHashMap FuncMap;

unsigned long hasFunc(FuncMap *m, char *str)		{ return mapHash(m, str); }
FuncMap initFuncs(unsigned long cap)			{ return initHashMap(cap); }
void   putFunc(FuncMap *m, char *key, void *value)	{ hashMapPut(m, key, value); }
void  *getFunc(FuncMap *m, char *key)			{ return hashMapGet(m, key); }
void   freeFuncs(FuncMap *m)				{ hashMapFreeMap(m); }
