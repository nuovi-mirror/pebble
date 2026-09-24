#include "instructionmapper.h"

#include "allocator.h"
#include "hashmap.h"

unsigned long hashInstruction(InstructionMap* m, const char* str) {
  return mapHash(m, str);
}

InstructionMap initInstructionMap(unsigned long cap, Arena* persistAlloc) {
  return initHashMap(cap, persistAlloc);
}

void putInstruction(InstructionMap* m, const char* key, const void* value,
                    Arena* persistAlloc) {
  hashMapPut(m, key, value, persistAlloc);
}

void* getInstruction(InstructionMap* m, const char* key) {
  return hashMapGet(m, key);
}

void freeInstructionMap(InstructionMap* m) { hashMapFreeMap(m); }
