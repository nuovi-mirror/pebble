#ifndef STATE_INSTRUCTOINMAPPER_H_
#define STATE_INSTRUCTOINMAPPER_H_

#include "hashmap.h"

typedef SHashMap InstructionMap;

unsigned long hashInstruction
(InstructionMap *m, const char *str);

InstructionMap initInstructionMap
(unsigned long cap);

void putInstruction
(InstructionMap *m, const char *key, const void *value);

void *getInstruction
(InstructionMap *m, const char *key);

void freeInstructionMap
(InstructionMap *m);

#endif
