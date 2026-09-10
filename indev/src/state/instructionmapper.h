#ifndef STATE_INSTRUCTOINMAPPER_H_
#define STATE_INSTRUCTOINMAPPER_H_

#include "hashmap.h"

typedef SHashMap InstructionMap;

unsigned long hashInstruction
(InstructionMap *m, char *str);

InstructionMap initInstructionMap
(unsigned long cap);

void putInstruction
(InstructionMap *m, char *key, void *value);

void *getInstruction
(InstructionMap *m, char *key);

#endif
