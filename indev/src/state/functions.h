#ifndef STATE_FUNCTIONS_H_
#define STATE_FUNCTIONS_H_

#include "hashmap.h"

typedef SHashMap FuncMap;

unsigned long hasFunc
(FuncMap *m, char *str);

FuncMap initFuncs
(unsigned long cap);

void putFunc
(FuncMap *m, char *key, void *value);

void *getFunc
(FuncMap *m, char *key);

void freeFuncs
(FuncMap *m);

#endif
