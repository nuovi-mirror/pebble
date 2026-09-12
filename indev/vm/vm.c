/* 
 * include this to get the main VM entry point
 * run vmmain to run the VM 
 * main itself is owned by the platform layer 
 */

#include "vm.h"
#include "entry.h"
#include "print.h"
#include "snprint.h"
#include "copystr.h"
#include "getstrlen.h"
#include "findnewline.h"
#include "cmpstr.h"
#include "copymem.h"
#include "readfile.h"
#include "skipspace.h"
#include "exitproc.h"
#include "setmem.h"
#include "allocator.h"
#include "limits.h"
#include "variables.h"
#include "values.h"
#include "functions.h"
#include "instructions.h"
#include "expressions.h"
#include "evaluator.h"
#include "instructionmapper.h"
#include "main.h"

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
	static unsigned long current_instruction_cachesize;

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
	/* XXX third operand unused for now */

	/* the instruction has been made 
	 * the instruction does not exist on the mapper yet 
	 * so we map it for future use */

	/* but only if we did not hit the max */
	if (current_instruction_cachesize <= limits_instructions_maxcache)
	{
		char *buf = alloc(persistAlloc, sizeof(Instruction));
		copymem(&instr, buf, sizeof(instr));
		putInstruction(instructionMap, line, buf);
	}

	return instr;
}

/* helper to find the end of a function during definition */
unsigned long findend
(Instruction *program, unsigned long instruction_count, unsigned long start)
{
	unsigned long depth = 1;
	unsigned long pc = start + 1;
 
	for (; pc < instruction_count; pc++)
	{
		if (program[pc].Opcode == Opcode_Func)
			depth++;
		else if (program[pc].Opcode == Opcode_End)
		{
			depth--;
			if (depth == 0)
				return pc;
		}
	}

	print("ERROR: HELPER FINDEND: NO MATCHING END FOUND!\n");
	exitproc(1);
}

/* helpers to resolve addressing modes */
/* XXX audit this */
Value resolve_literal
(char *buff, unsigned long buffsize, Value *val, VarMap *vars, Arena *tempAlloc)
{
    if (val->Type == type_expr)
    {
        if (val->as.expr == NULL)
        {
            print("ERROR: HELPER RESOLVE_LITERAL: NULL EXPRESSION!\n");
            exitproc(1);
        }

        *val = evalexprnode(val->as.expr, vars);
        return *val;
    }

    char buf[32];
    const char *text;

    if (val->Type == type_str)
    {
        text = val->as.str;
    }
    else
    {
        valuetostr(buf, sizeof(buf), *val);
        text = buf;
    }

    int ok;
    ExprNodeData tree = str2expr(text, &ok, tempAlloc);

    if (!ok)
    {
        print("ERROR: HELPER RESOLVE_LITERAL: MALFORMED EXPRESSION!\n");
        exitproc(1);
    }

    *val = evalexprdata(tree, vars);
    return *val;
}
	
Value resolve_forced_eval
(char *buff, unsigned long buffsize, Value *val, VarMap *vars, Arena *tempAlloc)
{
	valuetostr(buff, buffsize, *val);
	Value *var = getVar(vars, buff);

	if (var != NULL)
		return resolve_literal(buff, buffsize, var, vars, tempAlloc);

	return resolve_literal(buff, buffsize, val, vars, tempAlloc);
}

unsigned long resolvefunction
(unsigned long pc, FuncMap *funcs, Stack *stack, Instruction *program, Arena *scratchAlloc,
 const char *funcname, unsigned long instruction_count)
{
	unsigned long *retpc = getFunc(funcs, funcname);
	if (retpc == NULL)
	{
		print("ERROR: HELPER RESOLVEFUNC: UNKNOWN FUNCTION!\n");
		exitproc(1);
	}
	
	int tpos = (pc + 1 < instruction_count)
		&& program[pc + 1].Opcode == Opcode_End;
	int scall = (stack->count != 0) 
		&& cmpstr(stack->items[stack->count - 1].funcname, funcname) == 0;

	if (tpos && scall) /* tail-call */
		return *retpc;

	/* is not a tail call */
	StackFrame *frame = alloc(scratchAlloc, sizeof(StackFrame));
	frame->return_pc = pc + 1; /* since we are on the func instruction */
	frame->funcname = funcname;
	pushframe(stack, frame); 
	return *retpc;
}
	

/* instruction interpreter - instruction-by-instruction loop of execution */
unsigned long interpret
(Instruction *instr, Instruction *program,  unsigned long instruction_count, VarMap *vars, Stack *stack, 
 FuncMap *funcs, unsigned long pc, Arena *persistAlloc, Arena *scratchAlloc, Arena *tempAlloc) 
{
	switch (instr->Opcode) {
		case Opcode_New: {
			char *dest = alloc(tempAlloc, limits_instructions_varnamesize);
			Value val;

			switch (instr->FirstOperand.Addressing) {
				case addrmode_bare:
					valuetostr(dest, limits_instructions_varnamesize, instr->FirstOperand.Data);
					break;
				case addrmode_true_literal:
					valuetostr(dest, limits_instructions_varnamesize, instr->FirstOperand.Data);
					break;
				case addrmode_literal: {
					char *buf = alloc(tempAlloc, limits_instructions_varnamesize);
					val = resolve_literal(buf, limits_instructions_varnamesize, 
							&instr->FirstOperand.Data, vars, tempAlloc);
					valuetostr(dest, limits_instructions_varnamesize, val);
					break;
				}
				case addrmode_forced_eval: {
					char *buf = alloc(tempAlloc, limits_instructions_varnamesize);
					val = resolve_forced_eval(buf, limits_instructions_varnamesize, 
							&instr->FirstOperand.Data, vars, tempAlloc);
					valuetostr(dest, limits_instructions_varnamesize, val);
					break;
				}
	
				/* XXX handle addressing mode pointer */
				default:
					print("ERROR: VM: INTERPRETER: NEW: UNKNOWN ADDRESSING MODE ON OPERAND ONE\n");
					exitproc(1);
					break;
			}

			switch (instr->SecondOperand.Addressing) {
				case addrmode_true_literal:
					val = instr->SecondOperand.Data;
					break;
				case addrmode_literal: {
					char *buf = alloc(tempAlloc, limits_instructions_varnamesize);
					val = resolve_literal(buf, limits_instructions_varnamesize, 
						&instr->SecondOperand.Data, vars, tempAlloc);
					break;
				}

				case addrmode_forced_eval: {
					char *buf = alloc(tempAlloc, limits_instructions_varnamesize);
					val = resolve_forced_eval(buf, limits_instructions_varnamesize, 
						&instr->SecondOperand.Data, vars, tempAlloc);
					break;
				}

				/* XXX handle addressing mode pointer for this */
				/* XXX handle addressing mode bare for this */
				default:
					print("ERROR: VM: INTERPRETER: NEW: UNKNOWN ADDRESSING MODE ON OPERAND TWO\n");
					exitproc(1);
					break;
			}

			Value *valptr = alloc(persistAlloc, sizeof(val));
			*valptr = val;
			unsigned long namelen = getstrlen(dest);
			char *name = alloc(persistAlloc, namelen + 1); /* +1 for the null terminator */
			copymem(dest, name, namelen);
			name[namelen] = '\0';
		
			putVar(vars, name, valptr);

			break;
		}

		case Opcode_Func: {
			char *funcname = alloc(persistAlloc, limits_functions_namesize);
			valuetostr(funcname, limits_functions_namesize, instr->FirstOperand.Data);
			switch(instr->FirstOperand.Addressing) {
				case addrmode_bare: {
					unsigned long *lpc = alloc(persistAlloc, sizeof(long));
					*lpc = pc + 1; /* since we are on the func instruction */
					putFunc(funcs, funcname, lpc); /* place the function onto the index */
					break;
				}

				/* XXX handle addressing mode literal */
				/* XXX handle addressing mode true_literal */
				/* XXX handle addressing mode pointer */
				/* XXX handle addressing mode forced_eval */
				default: 
					print("ERROR: INTERPRETER: FUNC: UNSUPPORTED ADDRESSING MODE!\n");
					exitproc(1);
					break;
			}
			return findend(program, instruction_count, pc) + 1; /* find the End statement for this
									     * function and goto it */
			break;
		}

		case Opcode_If: {
			char *funcname = alloc(persistAlloc, limits_functions_namesize);
			Value result;

			switch(instr->FirstOperand.Addressing) {
				case addrmode_bare: 
					valuetostr(funcname, limits_functions_namesize, instr->FirstOperand.Data);
					break;


				/* XXX handle addressing mode literal */
				/* XXX handle addressing mode true_literal */
				/* XXX handle addressing mode pointer */
				/* XXX handle addressing mode forced_eval */
				default:
					print("ERROR: INTERPRETER: IF: UNSUPPORTED ADDRESSING MODE!\n");
					exitproc(1);
					break;
			}

			switch(instr->SecondOperand.Addressing) {
				case addrmode_literal: {
					char *buff = alloc(tempAlloc, limits_functions_namesize);
					result = resolve_literal(buff, limits_functions_namesize, 
							&instr->SecondOperand.Data, vars, tempAlloc);
					break;
				}
	
				/* XXX handle addressing mode true_literal */
				/* XXX handle addressing mode bare */
				/* XXX handle addressing mode pointer */
				/* XXX handle addressing mode forced_eval */
				default:
					print("ERROR: INTERPRETER: IF: UNSUPPORTED ADDRESSING MODE!\n");
					exitproc(1);
					break;

			}		
			if (valuetoword(result, vars).as.word == 0)
				return resolvefunction(pc, funcs, stack, program, scratchAlloc, funcname, instruction_count);
			else
				return pc + 1;

			break;
		}

		case Opcode_Call: {
			char *funcname = alloc(persistAlloc, limits_functions_namesize);

			switch(instr->FirstOperand.Addressing) {
				case addrmode_bare:
					valuetostr(funcname, limits_functions_namesize, instr->FirstOperand.Data);
					break;

				/* XXX handle addressing mode literal */
				/* XXX handle addressing mode true_literal */
				/* XXX handle addressing mode pointer */
				/* XXX handle addressing mode forced_eval */
				default:
					print("ERROR: INTERPRETER: CALL: UNSUPPORTED ADDRESSING MODE!\n");
					exitproc(1);
					break;
			}
			return resolvefunction(pc, funcs, stack, program, scratchAlloc, funcname, instruction_count);
			break;
		}

		case Opcode_Return:
			/* XXX add Return support */
			break;

		case Opcode_End:
			if (stack->count != 0) 
			{
				StackFrame frame = popframe(stack);
				/* XXX remove this line return pc = frame.return_pc; */
				return frame.return_pc;
			}
			break;

		case Opcode_Escape:
			/* XXX add Escape support */
			break;

		/* internal opcodes */
		case Opcode_Internal_PRINT: {
			char buf[32];

			print("_PRINT  (INTERNAL INSTRUCTION): ");
			print("TYPE: ");
			
			switch (instr->FirstOperand.Data.Type) {
				case type_word:	print("WORD,  DATA: ");	valuetostr(buf, sizeof(buf), instr->FirstOperand.Data);	break;
				case type_sword:print("SWORD, DATA: "); valuetostr(buf, sizeof(buf), instr->FirstOperand.Data); break;
				case type_str:	print("STR,   DATA: ");	copystr(instr->FirstOperand.Data.as.str, buf);		break;
				case type_flt:	print("FLT,   DATA: ");	valuetostr(buf, sizeof(buf), instr->FirstOperand.Data);	break;
				case type_expr: print("ERROR: THIS CANNOT HANDLE EXPRESSIONS!\n"); exitproc(1); 			break;
				case type_null: print("ERROR: THIS CANNOT HANDLE NULL TYPES!\n"); exitproc(1); 			break;
			}
			print(buf);
			print("\n");

			break;
		}

		case Opcode_Internal_PRINT2: {
			char *buf = alloc(scratchAlloc, limits_instructions_varnamesize);

			valuetostr(buf, limits_instructions_varnamesize, instr->FirstOperand.Data);
			Value *stored = getVar(vars, buf);

			if (stored == NULL)
			{
				print("ERROR: VM: INTERPRETER: _PRINT2: VARIABLE DOES NOT EXIST!\n");
				exitproc(1);
			}
			
			Value data = *stored;

			print("_PRINT2 (INTERNAL INSTRUCTION): ");
			print("TYPE: ");
	
			switch (data.Type) {
				case type_word:	print("WORD,  DATA: ");	valuetostr(buf, sizeof(buf), data);	break;
				case type_sword:print("SWORD, DATA: "); valuetostr(buf, sizeof(buf), data);	break;
				case type_str:	print("STR,   DATA: ");	copystr(data.as.str, buf);		break;
				case type_flt:	print("FLT,   DATA: ");	valuetostr(buf, sizeof(buf), data);	break;
				case type_expr: print("ERROR: THIS CANNOT HANDLE EXPRESSIONS!\n"); exitproc(1);	break;
				case type_null: print("ERROR: THIS CANNOT HANDLE NULL TYPES!\n"); exitproc(1); 	break;
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
	
	/* no fancy control flow needed here - incriment the pc */
	return pc + 1;
}

/* Args and Stack should be defined by the platform entry code 
 * which will include this file */
int vmmain
(Args cliargs, Stack *stack) 
{
	/* init */
	struct Arena *tempAlloc 	= initAlloc(limits_allocator_temp_maxmem);
	struct Arena *persistAlloc 	= initAlloc(limits_allocator_persist_maxmem);
	struct Arena *scratchAlloc 	= initAlloc(limits_allocator_scratch_maxmem);
	struct Arena *IRAlloc 		= initAlloc(limits_instructions_maxbuffersize);

	VarMap vars = initVars(limits_variables_max);
	FuncMap funcs = initFuncs(limits_functions_max);
	InstructionMap instructionMap = initInstructionMap(limits_instructions_maxcache);

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
	resetAllocator(tempAlloc);

	/* execution */
	unsigned long pc = 0;
	while (pc < current_instruction_count)
	{
		pc = interpret(&program[pc], program, current_instruction_count, &vars, stack, &funcs, pc, 
				persistAlloc, scratchAlloc, tempAlloc);
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
