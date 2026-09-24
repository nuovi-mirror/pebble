#ifndef h_instructionmapper_
#define h_instructionmapper_
#include "allocator.h"
#include "hashmap.h"

typedef SHashMap InstructionMap;

unsigned long hashInstruction (InstructionMap *m, const char *str);
InstructionMap initInstructionMap (unsigned long cap, Arena *persistAlloc);
void putInstruction (InstructionMap *m, const char *key, const void *value, Arena *persistAlloc);
void *getInstruction (InstructionMap *m, const char *key);
void freeInstructionMap (InstructionMap *m);
#endif
