#include "values.h"

typedef enum {
	literal,
	true_literal,
	forced_eval,
	bare,
	pointer,
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
	Data Value;
	Type InstructionType;
	Addressing InstructionAddressingMode;
}

typedef struct Instruction {
	Opcode InstructionOpcode;
	FirstOperand InstructionOperand;
	SecondOperand InstructionOperand;
	ThirdOperand InstructionOperand;
} Instruction;
