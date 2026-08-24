#pragma once

#include "values.h"

typedef enum {
	addrmode_literal,
	addrmode_true_literal,
	addrmode_forced_eval,
	addrmode_bare,
	addrmode_pointer,
} InstructionAddressingMode;

typedef enum {
	Opcode_New,
	Opcode_Func,
	Opcode_If,
	Opcode_Call,
	Opcode_Return,
	Opcode_Escape,
	Opcode_End,
} InstructionOpcode;

typedef struct InstructionOperand {
	Value Data;
	InstructionAddressingMode Addressing;
} InstructionOperand;

typedef struct {
	const char *name;
	InstructionOpcode op;
} InstructionOpcodeEntry;

static const InstructionOpcodeEntry InstructionOpcodeEntryTable[] = {
	{ "New", Opcode_New },
	{ "Func", Opcode_Func },
	{ "If", Opcode_If },
	{ "Call", Opcode_Call },
	{ "Return", Opcode_Return },
	{ "End", Opcode_End },
	{ "Escape", Opcode_Escape },
};

#define INSTRUCTION_OPCODE_TABLE_LEN (sizeof(InstructionOpcodeEntryTable) / sizeof(InstructionOpcodeEntryTable[0]))

typedef struct Instruction {
	InstructionOpcode Opcode;
	InstructionOperand FirstOperand;
	InstructionOperand SecondOperand;
	InstructionOperand ThirdOperand;
} Instruction;

int lookupopcode(const char *word, InstructionOpcode *out) {
	for (size_t i = 0; i < INSTRUCTION_OPCODE_TABLE_LEN; i++) {
		if (cmpstr(InstructionOpcodeEntryTable[i].name, word) == 0) {
			*out = InstructionOpcodeEntryTable[i].op;
			return 1;
		}
	}

	return 0;
}
