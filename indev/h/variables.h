#ifndef STATE_VARIABLES_H_
#define STATE_VARIABLES_H_

#include "hashmap.h"

typedef	SHashMap VarMap;

static unsigned long hashVar 
(VarMap *m, const char *str);

VarMap initVars 
(unsigned long cap);

void putVar 
(VarMap *m, const char *key, const void *value);

void *getVar 
(VarMap *m, const char *key);

void freeVars 
(VarMap *m);

#endif
