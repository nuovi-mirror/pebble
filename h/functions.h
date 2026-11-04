#ifndef h_functions_
#define h_functions_
#include "allocator.h"
#include "hashmap.h"

typedef SHashMap FuncMap;

unsigned long hasFunc (FuncMap *m, const char *str);
FuncMap initFuncs (unsigned long cap, Arena *persistAlloc);
void putFunc (FuncMap *m, const char *key, const void *value, Arena *persistAlloc);
void *getFunc (FuncMap *m, const char *key);
void freeFuncs (FuncMap *m);
#endif
