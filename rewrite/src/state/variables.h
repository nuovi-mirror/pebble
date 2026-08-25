#pragma once

#include "hashmap.h"

typedef SHashMap VarMap;

static size_t hashVar(VarMap *m, char *str)              { return mapHash(m, str); }
static VarMap initVars(size_t cap)                       { return initHashMap(cap); }
static void   putVar(VarMap *m, char *key, void *value)  { hashMapPut(m, key, value); }
static void  *getVar(VarMap *m, char *key)               { return hashMapGet(m, key); }
static void   freeVars(VarMap *m)                        { hashMapFreeMap(m); }
