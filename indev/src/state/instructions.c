#include "instructions.h"
#include "values.h"
#include "cmpstr.h"

int lookupopcode(const char *word, InstructionOpcode *out) {
	for (unsigned long i = 0; i < INSTRUCTION_OPCODE_TABLE_LEN; i++) {
		if (cmpstr(InstructionOpcodeEntryTable[i].name, word) == 0) {
			*out = InstructionOpcodeEntryTable[i].op;
			return 1;
		}
	}

	return 0;
}
