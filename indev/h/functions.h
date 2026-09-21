#ifndef STATE_FUNCTIONS_H_
#define STATE_FUNCTIONS_H_

#include "hashmap.h"

typedef SHashMap FuncMap;

unsigned long hasFunc
(FuncMap *m, const char *str);

FuncMap initFuncs
(unsigned long cap);

void putFunc
(FuncMap *m, const char *key, const void *value);

void *getFunc
(FuncMap *m, const char *key);

void freeFuncs
(FuncMap *m);

#endif
