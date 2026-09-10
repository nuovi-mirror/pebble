#include "instructionmapper.h"
#include "hashmap.h"

unsigned long hashInstruction(InstructionMap *m, char *str)		{ return mapHash(m, str); }
InstructionMap initInstructionMap(unsigned long cap)			{ return initHashMap(cap); }
void   putInstruction(InstructionMap *m, char *key, void *value)		{ hashMapPut(m, key, value); }
void  *getInstruction(InstructionMap *m, char *key)			{ return hashMapGet(m, key); }
