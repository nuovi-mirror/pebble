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
#include "../platform/use/setmem.h"

#include "../allocator/allocator.h"

#include "../state/limits.h"
#include "../state/variables.h"
#include "../state/functions.h"
#include "../state/values.h"
#include "../state/instructions.h"
#include "../state/expressions.h"
#include "../state/evaluator.h"
#include "../state/instructionmapper.h"

/* used to parse an instruction operand - guesses the type
 * and addressing mode */
int parseoperand
(char **cursor, InstructionOperand *out, Arena *persistAlloc) 
{
	char *p = skipspace(*cursor);
	if (p == NULL)
		return 0;

	char delim = 0;

	switch (*p) 
	{
		case '"': out->Addressing = addrmode_literal; delim = '"'; break;
		case '\'': out->Addressing = addrmode_true_literal; delim = '\''; break;
		case '{': out->Addressing = addrmode_forced_eval; delim = '}'; break;
		case '<': out->Addressing = addrmode_pointer; delim = '>'; break;
		default: out->Addressing = addrmode_bare; delim = 0; break;
	}

	char *start;

	if (delim) 
	{
		p++;					/* skip opening delim */
		start = p;

		while (*p != delim && *p != '\0')
			p++;

		if (*p != delim) 
		{
			print("ERROR: VM: MAKEIR: UNDETERMINED OPERAND\n");
			exitproc(1);
		}

		*p = '\0';				/* terminate operand text */
		p++;					/* move past it for next parse */
	} 
	else 
	{
		start = p;

		while (*p != ' ' && *p != '\t' && *p != '\0') 
			p++;

		if (*p != '\0') 
		{
			*p = '\0';
			p++;
		}
	}

	out->Data = guessvaluetypeorexpr(start, persistAlloc);

	*cursor = p;
	return 1;
}

/* used to generate and optimize instruction intermediate
 * representation (IR) */
Instruction makeIR
(char *line, Arena *persistAlloc, InstructionMap *instructionMap) 
{	
	Instruction *instrFromMap = getInstruction(instructionMap, line);
	if (instrFromMap != NULL)
		return *instrFromMap;
		/* this is safe since it should have already been placed on
		 * the persistent allocator */

	Instruction instr = { 0 };
	char *cursor = line;

	char *p = skipspace(cursor);
	if (p == NULL) 
	{
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

	parseoperand(&cursor, &instr.FirstOperand, persistAlloc);
	parseoperand(&cursor, &instr.SecondOperand, persistAlloc);
	/* third operand unused for now */

	/* the instruction has been made 
	 * the instruction does not exist on the mapper yet 
	 * so we map it for future use */

	char *buf = alloc(persistAlloc, sizeof(Instruction));
	copymem(&instr, buf, sizeof(instr));
	putInstruction(instructionMap, line, buf);

	return instr;
}

/* instruction interpreter - instruction-by-instruction loop of execution */
void interpret
(Instruction *instr, VarMap *vars, Arena *persistAlloc, Arena *scratchAlloc, Arena *tempAlloc) 
{
	switch (instr->Opcode) {
		case Opcode_New: {
			char dest[32];
			char data[32];
			Value val;

			switch (instr->FirstOperand.Addressing) {
				case addrmode_bare:
					valuetostr(dest, sizeof(dest), instr->FirstOperand.Data);
					break;
				case addrmode_true_literal:
					valuetostr(dest, sizeof(dest), instr->FirstOperand.Data);
					break;
				default:
					print("ERROR: VM: INTERPRETER: NEW: UNKNOWN ADDRESSING MODE ON OPERAND ONE\n");
					exitproc(1);
			}

			switch (instr->SecondOperand.Addressing) {
				case addrmode_true_literal:
					val = instr->SecondOperand.Data;
					break;
				case addrmode_literal: {
					if (instr->SecondOperand.Data.Type == type_expr)
						val = evalexprnode(instr->SecondOperand.Data.as.expr, vars);
					else 
					{
						char buf[32];
						const char *text;
						
						if (instr->SecondOperand.Data.Type == type_str)
							text = instr->SecondOperand.Data.as.str;
						else 
						{
							valuetostr(buf, sizeof(buf), 
									instr->SecondOperand.Data);
							text = buf;
						}

						int ok;
						ExprNodeData tree = str2expr(text, &ok, tempAlloc);

						if (!ok)
						{
							print("ERROR: VM: INTERPRETER: NEW: MALFORMED EXPRESSION!\n");
							exitproc(1);
						}

						val = evalexprdata(tree, vars);

					}

					break;
				}

				case addrmode_forced_eval: {
					char buff[32];
					valuetostr(buff, sizeof(buff), instr->SecondOperand.Data);
					Value *var = { 0 };
					
					if (var == NULL)
						var = &instr->SecondOperand.Data;
					else
						var = getVar(vars, buff);

					if (var->Type == type_expr)
						val = evalexprnode(var->as.expr, vars);
					else 
					{
						char buf[32];
						const char *text;
						
						if (var->Type == type_str)
							text = var->as.str;
						else 
						{
							valuetostr(buf, sizeof(buf), *var);
							text = buf;
						}

						int ok;
						ExprNodeData tree = str2expr(text, &ok, tempAlloc);

						if (!ok)
						{
							print("ERROR: VM: INTERPRETER: NEW: MALFORMED EXPRESSION!\n");
							exitproc(1);
						}

						val = evalexprdata(tree, vars);
					}

					break;
				}

				default:
					print("ERROR: VM: INTERPRETER: NEW: UNKNOWN ADDRESSING MODE ON OPERAND TWO\n");
					exitproc(1);
			}

			Value *valptr = alloc(persistAlloc, sizeof(val));
			*valptr = val;
			unsigned long namelen = getstrlen(dest);
			char *name = alloc(persistAlloc, namelen + 1); /* +1 for the null terminator */
			copymem(name, dest, namelen);
			name[namelen] = '\0';
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
			
			switch (instr->FirstOperand.Data.Type) {
				case type_word:		print("WORD,  DATA: ");		valuetostr(buf, sizeof(buf), instr->FirstOperand.Data);	break;
				case type_sword:	print("SWORD, DATA: "); 	valuetostr(buf, sizeof(buf), instr->FirstOperand.Data); break;
				case type_str:		print("STR,   DATA: ");		copystr(instr->FirstOperand.Data.as.str, buf);		break;
				case type_flt:		print("FLT,   DATA: ");		valuetostr(buf, sizeof(buf), instr->FirstOperand.Data);	break;
			}
			print(buf);
			print("\n");

			break;
		}

		case Opcode_Internal_PRINT2: {
			char buf[32];

			Value *stored = getVar(vars, instr->FirstOperand.Data.as.str);
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
		case Opcode_Internal_GETMEM: {
			char buf[32];

			print("_GETMEM (INTERNAL INSTRUCTION)\n");
			
			print("  PERMALLOC: USED: ");
			snprint(buf, sizeof(buf), "%lu", getAllocatorSizeUsed(persistAlloc));
			print(buf);
			setmem(buf, 0, sizeof(buf));
	
			print(", REMAIN: ");
			snprint(buf, sizeof(buf), "%lu", getAllocatorSizeRemaining(persistAlloc));
			print(buf);
			setmem(buf, 0, sizeof(buf));

			print(", MAX: ");
			snprint(buf, sizeof(buf), "%lu", persistAlloc->size);
			print(buf);
			setmem(buf, 0, sizeof(buf));

			print("\n");

			print("  TEMP: USED: ");
			snprint(buf, sizeof(buf), "%lu", getAllocatorSizeUsed(tempAlloc));
			print(buf);
			setmem(buf, 0, sizeof(buf));
	
			print(", REMAIN: ");
			snprint(buf, sizeof(buf), "%lu", getAllocatorSizeRemaining(tempAlloc));
			print(buf);
			setmem(buf, 0, sizeof(buf));

			print(", MAX: ");
			snprint(buf, sizeof(buf), "%lu", tempAlloc->size);
			print(buf);
			setmem(buf, 0, sizeof(buf));

			print("\n");

			print("  SCRATCH: USED: ");
			snprint(buf, sizeof(buf), "%lu", getAllocatorSizeUsed(scratchAlloc));
			print(buf);
			setmem(buf, 0, sizeof(buf));
	
			print(", REMAIN: ");
			snprint(buf, sizeof(buf), "%lu", getAllocatorSizeRemaining(scratchAlloc));
			print(buf);
			setmem(buf, 0, sizeof(buf));

			print(", MAX: ");
			snprint(buf, sizeof(buf), "%lu", scratchAlloc->size);
			print(buf);
			setmem(buf, 0, sizeof(buf));

			print("\n");

			break;
		}
		
		case Opcode_Internal_NOP: 
			/* do nothing */
			break;
	}
}

int vmmain(Args cliargs, Stack *stack) {

	struct Arena *tempAlloc 	= initAlloc(limits_allocator_temp_maxmem);
	struct Arena *persistAlloc 	= initAlloc(limits_allocator_persist_maxmem);
	struct Arena *scratchAlloc 	= initAlloc(limits_allocator_scratch_maxmem);
	struct Arena *IRAlloc 		= initAlloc(limits_instructions_maxbuffersize);

	VarMap vars = initVars(limits_variables_max);
	InstructionMap instructionMap = initInstructionMap(4096);

	char *file = alloc(tempAlloc, limits_misc_maxfilebuffersize);
	char *filedata = readfile(cliargs.values[1], file, limits_misc_maxfilebuffersize);

	if (filedata == NULL )
	{
		print("ERROR: VM: INIT: CANNOT OPEN SPECIFIED FILE!\n");
		exitproc(1);
	}

	unsigned long current_instruction_count = 0;
	char *line;

	unsigned long current_instruction_buffersize = limits_instructions_initbuffersize;

	Instruction *program = alloc(IRAlloc, limits_instructions_maxbuffersize);

	/* startup */
	while ((line = findnewline(&filedata)) != NULL)
	{
		if (current_instruction_count >= limits_instructions_max)
		{
			print("ERROR: LIMITS: INSTRUCTION CAP REACHED!\n");
			exitproc(1);
		}

		char *p = skipspace(line);

		if (p == NULL)
			continue;

		program[current_instruction_count] = makeIR(line, persistAlloc, &instructionMap);
		current_instruction_count++;
	}

	/* free buffer holding file */
	resetAllocator(scratchAlloc);

	/* execution */
	for (unsigned long pc = 0; pc < current_instruction_count; pc++)
	{
		interpret(&program[pc], &vars, persistAlloc, scratchAlloc, tempAlloc);
		resetAllocator(scratchAlloc);
		resetAllocator(tempAlloc);
	}

	/* clean-up */
	freeAllocator(persistAlloc);
	freeAllocator(tempAlloc);
	freeAllocator(scratchAlloc);
	freeAllocator(IRAlloc);

	return 0;	
}
