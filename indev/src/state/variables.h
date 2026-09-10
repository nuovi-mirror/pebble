#ifndef STATE_VARIABLES_H_
#define STATE_VARIABLES_H_

#include "hashmap.h"
#include "variables.h"

typedef SHashMap VarMap;

static unsigned long hashVar
(VarMap *m, char *str);

VarMap initVars
(unsigned long cap);

void putVar
(VarMap *m, char *key, void *value);

void *getVar
(VarMap *m, char *key);

void freeVars
(VarMap *m);

#endif
