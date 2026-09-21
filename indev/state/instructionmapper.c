#include "instructionmapper.h"
#include "hashmap.h"

unsigned long hashInstruction
(InstructionMap *m, const char *str)		
{ return mapHash(m, str); }

InstructionMap initInstructionMap
(unsigned long cap)			
{ return initHashMap(cap); }

void putInstruction
(InstructionMap *m, const char *key, const void *value)	
{ hashMapPut(m, key, value); }

void *getInstruction
(InstructionMap *m, const char *key)			
{ return hashMapGet(m, key); }

void freeInstructionMap
(InstructionMap *m)				
{ hashMapFreeMap(m); }
