#ifndef STATE_VARIABLES_H_
#define STATE_VARIABLES_H_

#include "hashmap.h"
#include "allocator.h"

typedef	SHashMap VarMap;

/* XXX unused 
static unsigned long hashVar 
(VarMap *m, const char *str);
*/

VarMap initVars 
(unsigned long cap, Arena *persistAlloc);

void putVar 
(VarMap *m, const char *key, const void *value, Arena *persistAlloc);

void *getVar 
(VarMap *m, const char *key);

void freeVars 
(VarMap *m);

#endif
