#ifndef STATE_INSTRUCTOINMAPPER_H_
#define STATE_INSTRUCTOINMAPPER_H_

#include "hashmap.h"

typedef SHashMap InstructionMap;

static unsigned long hashInstruction(InstructionMap *m, char *str)		{ return mapHash(m, str); }
static InstructionMap initInstructionMap(unsigned long cap)			{ return initHashMap(cap); }
static void   putInstruction(InstructionMap *m, char *key, void *value)		{ hashMapPut(m, key, value); }
static void  *getInstruction(InstructionMap *m, char *key)			{ return hashMapGet(m, key); }

#endif
