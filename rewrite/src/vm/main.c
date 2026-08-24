#include "../platform/use/print.h"
#include "../platform/use/copystr.h"
#include "../platform/use/getstrlen.h"
#include "../platform/use/findnewline.h"
#include "../platform/use/readfile.h"
#include "../platform/use/skipspace.h"

#include "../allocator/allocator.h"

#include "../state/variables.h"
#include "../state/functions.h"
#include "../state/values.h"
#include "../state/instructions.h"

int parseoperand(char **cursor, InstructionOperand *out) {
	char *p = skipspace(*cursor);
	if (p == NULL)
		return 0;

	char delim = 0;

	switch (*p) {
		case '"': out->Addressing = addrmode_literal; delim = '"'; break;
		case '\'': out->Addressing = addrmode_true_literal; delim = '\''; break;
		case '{': out->Addressing = addrmode_forced_eval; delim = '}'; break;
		case '<': out->Addressing = addrmode_pointer; delim = '>'; break;
		default: out->Addressing = addrmode_bare; delim = 0; break;
	}

	char *start;

	if (delim) {
		p++;					/* skip opening delim */

		start = p;

		while (*p != delim && *p != '\0')
			p++;

		if (*p != delim) {
			print("ERROR: VM: MAKEIR: UNDETERMINED OPERAND\n");
			exit(1);
		}

		*p = '\0';				/* terminate operand text */
		p++;					/* move past it for next parse */
	} else {
		while (*p != ' ' && *p != '\t' && *p != '\0') 
			p++;

		if (*p != '\0') {
			*p = '\0';
			p++;
		}
	}

	out->Data.type = type_string;
	out->Data.as.str = start;

	*cursor = p;
	return 1;
}

Instruction makeIR(char *line) {
	Instruction instr = { 0 };
	char *cursor = line;

	char *p = skipspace(cursor);
	if (p == NULL) {
		print("ERROR: VM: MAKEIR: EMPTY LINE\n");
		exit(1);
	}

	char *opcode_start = p;
	while (*p != ' ' && *p != '\t' && *p != '\0')
		p++;

	if (*p != '\0') {
		*p = '\0';
		p++;
	}

	cursor = p;

	if (!lookupopcode(opcode_start, &instr.Opcode)) {
		print("ERROR: VM: MAKEIR: UNKNOWN OPCODE\n");
		exit(1);
	}

	parseoperand(&cursor, &instr.FirstOperand);
	parseoperand(&cursor, &instr.SecondOperand);
	/* third operand unused for now */

	return instr;
}

void interpret(Instruction instr, Map *vars) {

}

int main(void) {

	
	/* init */
	struct Arena *tempAlloc = initAlloc(512 * 1024); /* N kb */
	struct Arena *permAlloc = initAlloc(6 * 1024 * 1024); /* N mb */
	struct Arena *scratchAlloc = initAlloc(128 * 1024); /* N kb */

	Map vars = initVars(512); /* N total variables */
	/* end init */

	size_t freadsize = 512 * 1024;
	char *file = alloc(permAlloc, freadsize); 
							/* N bytes to use for the
							   file data (allocate
							   and read) */

	char *filedata = readfile("testfile", file, freadsize);
							/* pointer to file data */
	char *line;
	while ((line = findnewline(&filedata)) != NULL) {
		if (line[0] == '\0')
			continue;			/* skip empty lines */

		Instruction instr = makeIR(line);	/* parse source code */
		interpret(instr, &vars);		/* interpret bytecode */
	}

	return 0;
}


