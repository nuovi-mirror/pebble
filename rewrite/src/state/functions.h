#ifndef STATE_FUNCTIONS_H_
#define STATE_FUNCTIONS_H_

#include "hashmap.h"

typedef SHashMap FuncMap;

static unsigned long hasFunc(FuncMap *m, char *str)		{ return mapHash(m, str); }
static FuncMap initFuncs(unsigned long cap)			{ return initHashMap(cap); }
static void   putFunc(FuncMap *m, char *key, void *value)	{ hashMapPut(m, key, value); }
static void  *getFuncs(FuncMap *m, char *key)			{ return hashMapGet(m, key); }
static void   freeFuncs(FuncMap *m)				{ hashMapFreeMap(m); }

#endif
