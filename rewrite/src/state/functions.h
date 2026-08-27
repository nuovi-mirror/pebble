#pragma once

#include "instructions.h"

#include "../platform/use/lalloc.h"
#include "../platform/use/getstrlen.h"
#include "../platform/use/copystr.h"

#define InstrMax 4096 /* max number of instructions on the table */
#define FuncnameMaxLen 32 /* max number of characters in a function name */

typedef struct InstructionTable {
	Instruction items[InstrMax];
	unsigned long count;
} InstructionTable;

typedef SHashMap FuncIndex;

static FuncIndex initFuncIndex(unsigned long cap) { return initHashMap(cap); }

static void recordFirstInstruction(FuncIndex *m, InstructionTable *c, 
		char *funcname, Instruction instruction) {
	if (c->count >= InstrMax) { 
		print("ERROR: VM: FUNCTIONS: INSTRUCTION TABLE OVERFLOW!\n");
		exitproc(1);
	}

	if (getstrlen(funcname) >= FuncnameMaxLen) {
		print("ERROR: VM: FUNCTIONS: INSTRUCTION NAME TOO LONG!\n");
		exitproc(1);
	}

	c->items[c->count++] = instruction;

	Instruction *instructioncopy = lalloc(sizeof(Instruction));
	*instructioncopy = instruction;

	char *funcnamecopy = lalloc(getstrlen(funcname));
	copystr(funcname, funcnamecopy);

	hashMapPut(m, funcname, instructioncopy);
}

static void recordInstruction(InstructionTable *c, Instruction instruction) {
	if (c->count >= InstrMax) { 
		print("ERROR: VM: FUNCTIONS: INSTRUCTION TABLE OVERFLOW!\n");
		exitproc(1);
	}

	c->items[c->count++] = instruction;
}

static Instruction fetchInstruction(InstructionTable *c) {
	if (c->count <= 0) {
		print("ERROR: VM: FUNCTIONS: INSTRUCTION TABLE UNDERFLOW!\n");
		exitproc(1);
	}

	Instruction instruction = c->items[c->count - 1];
	return instruction;
}

static InstructionTable *initInstructionTable(void) {
	
	InstructionTable *table = lalloc(sizeof(InstructionTable));
	table->count = 0;

	return table;
}
