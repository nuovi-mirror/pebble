/* include this to get the main VM entry point */
/* run vmmain to run the VM */
/* main itself is owned by the platform layer */

#include "../platform/use/print.h"
#include "../platform/use/copystr.h"
#include "../platform/use/getstrlen.h"
#include "../platform/use/findnewline.h"
#include "../platform/use/readfile.h"
#include "../platform/use/skipspace.h"
#include "../platform/use/exitproc.h"

#include "../allocator/allocator.h"

#include "../state/variables.h"
#include "../state/functions.h"
#include "../state/values.h"
#include "../state/instructions.h"
#include "../state/expressions.h"
#include "../state/evaluator.h"

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
			exitproc(1);
		}

		*p = '\0';				/* terminate operand text */
		p++;					/* move past it for next parse */
	} else {
		start = p;

		while (*p != ' ' && *p != '\t' && *p != '\0') 
			p++;

		if (*p != '\0') {
			*p = '\0';
			p++;
		}
	}

	out->Data = guessvaluetypeorexpr(start);

	*cursor = p;
	return 1;
}

Instruction makeIR(char *line) {
	Instruction instr = { 0 };
	char *cursor = line;

	char *p = skipspace(cursor);
	if (p == NULL) {
		print("ERROR: VM: MAKEIR: EMPTY LINE\n");
		exitproc(1);
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
		exitproc(1);
	}

	parseoperand(&cursor, &instr.FirstOperand);
	parseoperand(&cursor, &instr.SecondOperand);
	/* third operand unused for now */

	return instr;
}

void interpret(Instruction instr, VarMap *vars, Arena *persistAlloc, Arena *scratchAlloc) {
	switch (instr.Opcode) {
		case Opcode_New: {
			char dest[32];
			char data[32];
			Value val;

			switch (instr.FirstOperand.Addressing) {
				case addrmode_bare:
					valuetostr(dest, sizeof(dest), instr.FirstOperand.Data);
					break;
				case addrmode_true_literal:
					valuetostr(dest, sizeof(dest), instr.FirstOperand.Data);
					break;
				default:
					print("ERROR: VM: INTERPRETER: NEW: UNKNOWN ADDRESSING MODE ON OPERAND ONE\n");
					exitproc(1);
			}

			switch (instr.SecondOperand.Addressing) {
				case addrmode_true_literal:
					val = instr.SecondOperand.Data;
					break;
				case addrmode_literal: {
					if (instr.SecondOperand.Data.Type == type_expr)
						val = evalexprnode(instr.SecondOperand.Data.as.expr);
					else 
					{
						char buf[32];
						const char *text;
						
						if (instr.SecondOperand.Data.Type == type_str)
							text = instr.SecondOperand.Data.as.str;
						else 
						{
							valuetostr(buf, sizeof(buf), 
									instr.SecondOperand.Data);
							text = buf;
						}

						int ok;
						ExprNodeData tree = str2expr(text, &ok);

						if (!ok)
						{
							print("ERROR: VM: INTERPRETER: NEW: MALFORMED EXPRESSION!\n");
							exitproc(1);
						}

						val = evalexprdata(tree);
					}

					break;
				}

				default:
					print("ERROR: VM: INTERPRETER: NEW: UNKNOWN ADDRESSING MODE ON OPERAND TWO\n");
					exitproc(1);
			}

			Value *valptr = alloc(persistAlloc, sizeof(val));
			*valptr = val;
			char *name = alloc(persistAlloc, getstrlen(dest));
			copymem(name, dest, getstrlen(dest));
			putVar(vars, name, valptr);
			break;
		}

		case Opcode_Func:
			/* handle Func */
			break;

		case Opcode_If:
			/* handle If */
			break;

		case Opcode_Call:
			/* handle Call */
			break;

		case Opcode_Return:
			/* handle Retrun */
			break;

		case Opcode_End:
			/* handle End */
			break;

		case Opcode_Escape:
			/* handle Escape */
			break;

		/* internal opcodes */

		/* big long comment to grab your attention
		 * LALALLALALALALALLALALLALALALALALLALALALA
		 * LALALLALALALALALLALALLALALALALALLALALALA
		 * LALALLALALALALALLALALLALALALALALLALALALA
		 * LALALLALALALALALLALALLALALALALALLALALALA
		 * LALALLALALALALALLALALLALALALALALLALALALA
		 * LALALLALALALALALLALALLALALALALALLALALALA
		 * LALALLALALALALALLALALLALALALALALLALALALA
		 * LALALLALALALALALLALALLALALALALALLALALALA
		 * LALALLALALALALALLALALLALALALALALLALALALA
		 * LALALLALALALALALLALALLALALALALALLALALALA
		 * LALALLALALALALALLALALLALALALALALLALALALA
		 * LALALLALALALALALLALALLALALALALALLALALALA
		 */

		/* kay, now that i have your attention, these
		 * next bits are internal / debug instructions
		 * DO NOT USE THESE UNLESS YOU KNOW WHAT YOU ARE DOING */
		

		case Opcode_Internal_PRINT: {
			char buf[32];

			print("_PRINT (INTERNAL INSTRUCTION): ");
			print("TYPE: ");
			
			switch (instr.FirstOperand.Data.Type) {
				case type_word:		print("WORD,  DATA: ");		valuetostr(buf, sizeof(buf), instr.FirstOperand.Data);	break;
				case type_sword:	print("SWORD, DATA: "); 	valuetostr(buf, sizeof(buf), instr.FirstOperand.Data); 	break;
				case type_str:		print("STR,   DATA: ");		copystr(instr.FirstOperand.Data.as.str, buf);		break;
				case type_flt:		print("FLT,   DATA: ");		valuetostr(buf, sizeof(buf), instr.FirstOperand.Data);	break;
			}
			print(buf);
			print("\n");

			break;
		}

		case Opcode_Internal_PRINT2: {
			char buf[32];

			Value *stored = getVar(vars, instr.FirstOperand.Data.as.str);
			Value data = *stored; /* assume it exists - breaks if it does not */
			/* this is fine since this is a debug instruction */
			print("_PRINT2 (INTERNAL INSTRUCTION): ");
			print("TYPE: ");
	
			switch (data.Type) {
				case type_word:		print("WORD,  DATA: ");		valuetostr(buf, sizeof(buf), data);		break;
				case type_sword:	print("SWORD, DATA: "); 	valuetostr(buf, sizeof(buf), data); 		break;
				case type_str:		print("STR,   DATA: ");		copystr(data.as.str, buf);			break;
				case type_flt:		print("FLT,   DATA: ");		valuetostr(buf, sizeof(buf), data);		break;
			}

			print(buf);
			print("\n");

			break;
		}

	}
}

int vmmain(Args cliargs, Stack *stack) {

	
	/* init */
	struct Arena *tempAlloc = initAlloc(512 * 1024); /* N kb */
	struct Arena *permAlloc = initAlloc(6 * 1024 * 1024); /* N mb */
	struct Arena *scratchAlloc = initAlloc(128 * 1024); /* N kb */

	VarMap vars = initVars(512); /* N total variables */
	/* end init */

	unsigned long freadsize = 512 * 1024;
	char *file = alloc(permAlloc, freadsize); 
							/* N bytes to use for the
							   file data (allocate
							   and read) */
	
	char *filedata = readfile(cliargs.values[1], file, freadsize);
							/* argument 0 passed to us from */
							/* the platform layer */
							/* pointer to file data */
	if (filedata == NULL) {
		print("VM: INIT: ERROR: CANNOT OPEN SPECIFIED FILE\n");
		exitproc(1);
	}

	char *line;
	while ((line = findnewline(&filedata)) != NULL) {
		if (line[0] == '\0')
			continue;			/* skip empty lines */

		Instruction instr = makeIR(line);	/* parse source code */
		interpret(instr, &vars, permAlloc, scratchAlloc);
							/* interpret bytecode */
	}

	return 0;
}
