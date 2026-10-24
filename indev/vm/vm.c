#include "vm.h"
#include "allocator.h"
#include "cmpstr.h"
#include "copymem.h"
#include "copystr.h"
#include "escapes.h"
#include "evaluator.h"
#include "expressions.h"
#include "ffi.h"
#include "functions.h"
#include "getstrlen.h"
#include "instructionmapper.h"
#include "instructions.h"
#include "limits.h"
#include "main.h"
#include "print.h"
#include "setmem.h"
#include "skipspace.h"
#include "snprint.h"
#include "values.h"
#include "variables.h"
#include "tasks.h"

/* used to parse an instruction operand - guesses the type and addressing mode */
int parseoperand 
(char **cursor, InstructionOperand *out, Arena *tempAlloc, Arena *persistAlloc)
{
	char *p = skipspace(*cursor);
	if (p == NULL) return 0;

	char delim = 0;

	switch (*p) 
	{
		case '"':
			out->Addressing = addrmode_literal;
			delim = '"';
			break;
	
		case '\'':
			out->Addressing = addrmode_true_literal;
			delim = '\'';
			break;
	
		case '{':
			out->Addressing = addrmode_forced_eval;
			delim = '}';
			break;
	
		case '<':
			out->Addressing = addrmode_pointer;
			delim = '>';
			break;

		case '[':
			out->Addressing = addrmode_indirect;
			delim = ']';
			break;
	
		default:
			out->Addressing = addrmode_bare;
			delim = 0;
			break;
	}

	char *start;

	if (delim) 
	{
		p++; /* skip opening delim */
		start = p;

		while (*p != delim && *p != '\0') p++;

		if (*p != delim) error(errfatal, errcodemalformedinput, "Undetermined operand when generating IR!");

		*p = '\0'; /* terminate operand text */
		p++;	   /* move past it for next parse */
	} else 
	{
		start = p;

		while (*p != ' ' && *p != '\t' && *p != '\0') p++;

		if (*p != '\0') 
		{
			*p = '\0';
			p++;
		}
	}

	out->Data = guessvaluetypeorexpr(start, tempAlloc, persistAlloc);

	*cursor = p;
	return 1;
}

Instruction *optimizeIR
(Instruction *program, unsigned long *instruction_count, Arena *tempAlloc, Arena *persistAlloc)
{
	/* remove nop instructions */
	unsigned long write = 0;
	for (unsigned long read = 0; read < *instruction_count; read++) 
	{
		if (program[read].Opcode == Opcode_Internal_NOP) continue;
	
		if (write != read)
			program[write] = program[read];

		write++;
	}

	*instruction_count = write;

	/* return optimized program */
	return program;
}

/* used to generate and optimize instruction intermediate representation (IR) */
Instruction makeIR 
(char *line, struct Limits *limits, Arena *tempAlloc, Arena *persistAlloc, InstructionMap *instructionMap)
{
	if (*line == '/' || *line == '#') return (struct Instruction){ Opcode_Internal_NOP };
	Instruction *instrFromMap = getInstruction(instructionMap, line);

	if (instrFromMap != NULL) return *instrFromMap;
	/* this is safe since it should have already been placed on
	 * the persistent allocator */

	char *cachekey = NULL;

	if (instructionMap->size < limits->instructions_maxcache)
	{
		cachekey = alloc(persistAlloc, getstrlen(line) + 1);
		copystr(line, cachekey);
	}

	Instruction instr = {0};
	char *cursor = line;

	char *p = skipspace(cursor);
	if (p == NULL) error(errfatal, errcodemalformedinput, "Empty line found when generating IR!");

	char *opcode_start = p;
	while (*p != ' ' && *p != '\t' && *p != '\0') p++;

	if (*p != '\0') 
	{
		*p = '\0';
		p++;
	}

	cursor = p;

	if (!lookupopcode(opcode_start, &instr.Opcode)) error(errfatal, errcodemalformedinput, "Unknown opcode found when generating IR!");

	parseoperand(&cursor, &instr.FirstOperand, tempAlloc, persistAlloc);
	parseoperand(&cursor, &instr.SecondOperand, tempAlloc, persistAlloc);
	/* XXX third operand unused for now */

	/* the instruction has been made the instruction does not exist on the mapper yet
	 * so we map it for future use */
	if (cachekey != NULL)
	{
		char *buf = alloc(persistAlloc, sizeof(Instruction));
		copymem(&instr, buf, sizeof(instr));
		putInstruction(instructionMap, cachekey, buf, persistAlloc);
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
		if (program[pc].Opcode == Opcode_Func) depth++;
		else if (program[pc].Opcode == Opcode_End) 
		{
			depth--;
			if (depth == 0) return pc;
		}
	}

	error(errfatal, errcodemalformedinput, "No matching End opcode found!");
	return 0; /* XXX never hit */
}

/* helpers to resolve addressing modes */
Value resolve_literal 
(char *buff, unsigned long buffsize, Value *val, VarMap *vars, Arena *tempAlloc, Arena *persistAlloc, struct Limits *limits)
{
	if (val->Type == type_expr) 
	{
		if (val->as.expr == NULL) 
			error(errfatal, errcodemalformedinput, "A null expression pointer was given!");

		return evalexprnode(val->as.expr, vars, persistAlloc, limits);
	}

	const char *text;

	if (val->Type == type_str) 
	{
		text = val->as.str;
	} else 
	{
		valuetostr(buff, buffsize, *val, limits);
		text = buff;
	}

	/* only actually try to parse it as an expression if it looks like one
	 * parse it normally if not */
	if (isexpression(text)) 
	{
		int ok;
		ExprNodeData tree = str2expr(text, &ok, tempAlloc, persistAlloc);

		if (ok && tree.Type == ExprDataNode && tree.Node != NULL) 
			return evalexprdata(tree, vars, persistAlloc, limits);
	}

	return *val;
}

Value resolve_forced_eval 
(char *buff, unsigned long buffsize, Value *val, VarMap *vars, Arena *tempAlloc, Arena *persistAlloc, limits)
{
	valuetostr(buff, buffsize, *val, limits);
	Value *var = getVar(vars, buff);

	if (var != NULL) return resolve_literal(buff, buffsize, var, vars, tempAlloc, persistAlloc, limits);
	return resolve_literal(buff, buffsize, val, vars, tempAlloc, persistAlloc, limits);
}

Value resolve_pointer 
(char *buff, unsigned long buffsize, Value *val, VarMap *vars, Arena *tempAlloc, Arena *persistAlloc, struct Limits *limits) 
{
	valuetostr(buff, buffsize, *val, limits);
	Value *valptr = getVar(vars, buff);
	
	if (valptr == NULL) error(errfatal, errcodemalformedinput, "Specified variable does not exist!");

	return *valptr;
}

unsigned long resolvefunction 
(unsigned long pc, FuncMap *funcs, Stack *stack, Instruction *program, Arena *scratchAlloc, const char *funcname,
	unsigned long instruction_count)
{
	unsigned long *retpc = getFunc(funcs, funcname);
	if (retpc == NULL) error(errfatal, errcodemalformedinput, "The specified function does not exist!");

	int tpos = (pc + 1 < instruction_count) && program[pc + 1].Opcode == Opcode_End;
	int scall = (stack->count != 0) && cmpstr(stack->items[stack->count - 1].funcname, funcname) == 0;
	if (tpos && scall) return *retpc; /* tail-call */

	/* is not a tail call */
	StackFrame *frame = alloc(scratchAlloc, sizeof(StackFrame));
	frame->return_pc = pc + 1; /* since we are on the func instruction */
	frame->funcname = funcname;
	pushframe(stack, frame);
	return *retpc;
}

unsigned long interpret 
(Instruction *instr, Instruction *program, unsigned long long seed, unsigned long instruction_count, struct Limits *limits, VarMap *vars, FFIvars *ffivars, Stack *stack, FuncMap *funcs, unsigned long pc, Arena *persistAlloc, Arena *scratchAlloc, Arena *tempAlloc, Scheduler *scheduler)
{
	switch (instr->Opcode) 
	{
		case Opcode_New: 
		{
			char *dest = alloc(tempAlloc, limits->instructions_varnamesize);
			Value val;

			switch (instr->FirstOperand.Addressing) 
			{
				case addrmode_bare:
					valuetostr(dest, limits->instructions_varnamesize, instr->FirstOperand.Data, limits);
					break;

				case addrmode_true_literal:
					valuetostr(dest, limits->instructions_varnamesize, instr->FirstOperand.Data, limits);
					break;

				case addrmode_literal: 
				{
					char *buf = alloc(tempAlloc, limits->instructions_varnamesize);
					val = resolve_literal(buf, limits->instructions_varnamesize, &instr->FirstOperand.Data, vars, tempAlloc, persistAlloc, limits);
					valuetostr(dest, limits->instructions_varnamesize, val, limits);
					break;
				}
				case addrmode_forced_eval: 
				{
					char *buf = alloc(tempAlloc, limits->instructions_varnamesize);
					val = resolve_forced_eval(buf, limits->instructions_varnamesize, &instr->FirstOperand.Data, vars, tempAlloc, persistAlloc, limits);
					valuetostr(dest, limits->instructions_varnamesize, val, limits);
					break;
				}

				case addrmode_indirect: 
				{
					char *buff = alloc(tempAlloc, limits->instructions_varnamesize);
					Value v = resolve_pointer(buff, limits->instructions_varnamesize, &instr->FirstOperand.Data, vars, tempAlloc, persistAlloc, limits);
					valuetostr(dest, limits->instructions_varnamesize, v, limits);
					break;
				}

				case addrmode_pointer: 
				{
					char *buff = alloc(tempAlloc, limits->instructions_varnamesize);
					Value v = resolve_pointer(buff, limits->instructions_varnamesize, &instr->FirstOperand.Data, vars, tempAlloc, persistAlloc, limits);
					valuetostr(dest, limits->instructions_varnamesize, v, limits);
					break;
				}

				default: 
					error(errfatal, errcodemalformedinput, "Unknown addressing mode!"); 
					break;
			}

			switch (instr->SecondOperand.Addressing) 
			{
				case addrmode_true_literal:
					val = instr->SecondOperand.Data;
					break;
				case addrmode_literal: 
				{
					char *buf = alloc(tempAlloc, limits->instructions_varnamesize);
					val = resolve_literal(buf, limits->instructions_varnamesize, &instr->SecondOperand.Data, vars, tempAlloc, persistAlloc, limits);
					break;
				}

				case addrmode_forced_eval: 
				{
					char *buf = alloc(tempAlloc, limits->instructions_varnamesize);
					val = resolve_forced_eval(buf, limits->instructions_varnamesize, &instr->SecondOperand.Data, vars, tempAlloc, persistAlloc, limits);
					break;
				}

				case addrmode_bare: 
				{
					char *buf = alloc(scratchAlloc, limits->instructions_varnamesize);
					valuetostr(buf, limits->instructions_varnamesize, instr->SecondOperand.Data, limits);
					Value *check = getVar(vars, buf);
					if (check == NULL) error(errfatal, errcodemalformedinput, "The specified variable does not exist!");
					val = *check;
					break;
				}

				case addrmode_pointer: 
				{
					char *buff = alloc(persistAlloc, limits->instructions_varnamesize);
					valuetostr(buff, limits->instructions_varnamesize, instr->SecondOperand.Data, limits);
					val = (Value){ .Type = type_str, .as.str = buff };
					break;
				}

				case addrmode_indirect: 
				{
					char *buff1 = alloc(persistAlloc, limits->instructions_varnamesize);
					char *buff2 = alloc(persistAlloc, limits->instructions_varnamesize);

					Value ref = resolve_pointer(buff1, limits->instructions_varnamesize, &instr->SecondOperand.Data, vars, tempAlloc, persistAlloc, limits);
					val = resolve_pointer(buff2, limits->instructions_varnamesize, &ref, vars, tempAlloc, persistAlloc, limits);
					
					break;
				}

				default:
					error(errfatal, errcodemalformedinput, 
						"The specified addressing mode is invalid!");
					break;
			}

			Value *existing = getVar(vars, dest);
			if (existing != NULL) *existing = val;

			else 
			{
				Value *valptr = alloc(persistAlloc, sizeof(val));
				*valptr = val;
				unsigned long namelen = getstrlen(dest);
				char *name = alloc(persistAlloc, namelen + 1); /* +1 for the null terminator */
				copymem(dest, name, namelen);
				name[namelen] = '\0';

				putVar(vars, name, valptr, persistAlloc);
			}

			break;
		}

		case Opcode_Func: 
		{
			char *funcname = alloc(persistAlloc, limits->functions_namesize);
			unsigned long *lpc = alloc(persistAlloc, 64);
			
			switch (instr->FirstOperand.Addressing) 
			{
				case addrmode_bare: 
				case addrmode_true_literal: 
				{
					valuetostr(funcname, limits->functions_namesize, instr->FirstOperand.Data, limits);
					break;
				}

				case addrmode_literal: 
				{
					char *buff = alloc(scratchAlloc, limits->functions_namesize);
					Value vptr = resolve_literal(buff, limits->functions_namesize, &instr->FirstOperand.Data, vars, tempAlloc, persistAlloc, limits);
					valuetostr(funcname, limits->functions_namesize, vptr, limits);
					break;
				}

				case addrmode_forced_eval: 
				{
					char *buff = alloc(scratchAlloc, limits->functions_namesize);
					Value vptr = resolve_forced_eval(buff, limits->functions_namesize, &instr->FirstOperand.Data, vars, tempAlloc, persistAlloc, limits);
					valuetostr(funcname, limits->functions_namesize, vptr, limits);
					break;
				}

				case addrmode_pointer: 
				case addrmode_indirect: 
				{
					char *buff = alloc(tempAlloc, limits->instructions_varnamesize);
					Value v = resolve_pointer(buff, limits->functions_namesize, &instr->FirstOperand.Data, vars, tempAlloc, persistAlloc, limits);
					valuetostr(funcname, limits->functions_namesize, v, limits);
					break;
				}

				default:
					error(errfatal, errcodemalformedinput, "The specified addressing mode is invalid!");
					break;
			}

			*lpc = pc + 1; /* since we are on the func instruction */
			putFunc(funcs, funcname, lpc, persistAlloc); /* place the function onto the index */
			return findend(program, instruction_count, pc) + 1; /* find the End statement */
			break;
		}

		case Opcode_If: 
		{
			char *funcname = alloc(scratchAlloc, limits->functions_namesize);
			Value result;

			switch (instr->FirstOperand.Addressing) 
			{
				case addrmode_bare:
				case addrmode_true_literal:
					valuetostr(funcname, limits->functions_namesize, instr->FirstOperand.Data, limits);
					break;

				case addrmode_literal: 
				{
					char *buff = alloc(scratchAlloc, limits->functions_namesize);
					Value vptr = resolve_literal(buff, limits->functions_namesize, &instr->FirstOperand.Data, vars, tempAlloc, persistAlloc, limits);
					valuetostr(funcname, limits->functions_namesize, vptr, limits);
					break;
				}

				case addrmode_forced_eval: 
				{
					char *buff = alloc(scratchAlloc, limits->functions_namesize);
					Value vptr = resolve_forced_eval(buff, limits->functions_namesize, &instr->FirstOperand.Data, vars, tempAlloc, persistAlloc, limits);
					valuetostr(funcname, limits->functions_namesize, vptr, limits);
					break;
				}

				case addrmode_pointer: 
				{
					char *buff = alloc(tempAlloc, limits->functions_namesize);
					Value v = resolve_pointer(buff, limits->functions_namesize, &instr->FirstOperand.Data, vars, tempAlloc, persistAlloc, limits);
					valuetostr(funcname, limits->functions_namesize, v, limits);
					break;
				}

				case addrmode_indirect: 
				{
					char *buff = alloc(tempAlloc, limits->functions_namesize);
					Value v = resolve_pointer(buff, limits->functions_namesize, &instr->FirstOperand.Data, vars, tempAlloc, persistAlloc, limits);
					valuetostr(funcname, limits->functions_namesize, v, limits);
					break;
				}

				default:
					error(errfatal, errcodemalformedinput, "The specified addressing mode is invalid!");
					break;
			}

			switch (instr->SecondOperand.Addressing) 
			{
				case addrmode_literal: 
				{
					char *buff = alloc(tempAlloc, limits->functions_namesize);
					result = resolve_literal(buff, limits->functions_namesize, &instr->SecondOperand.Data, vars, tempAlloc, persistAlloc, limits);
					break;
				}

				case addrmode_forced_eval: 
				{
					char *buff = alloc(tempAlloc, limits->functions_namesize);
					result = resolve_forced_eval(buff, limits->functions_namesize, &instr->SecondOperand.Data, vars, tempAlloc, persistAlloc, limits);
					break;
				}

				/* in this case, a pointer refers to a pointer to an
				 * expression, which is compatable with the forced_eavl
				 * addressing mode, so we can just copy that */
				case addrmode_pointer: 
				{
					char *buff = alloc(tempAlloc, limits->functions_namesize);
					result = resolve_forced_eval(buff, limits->functions_namesize, &instr->SecondOperand.Data, vars, tempAlloc, persistAlloc, limits);
					break;
				}

				case addrmode_indirect: 
				{
					char *buff = alloc(tempAlloc, limits->functions_namesize);
					result = resolve_forced_eval(buff, limits->functions_namesize, &instr->SecondOperand.Data, vars, tempAlloc, persistAlloc, limits);
					break;
				}

				case addrmode_bare: 
				{
					result = instr->SecondOperand.Data;
					break;
				}

				case addrmode_true_literal: 
				{
					result = instr->SecondOperand.Data;
					break;
				}

				default:
					error(errfatal, errcodemalformedinput, "The specified addressing mode is invalid!");
					break;
			}

			if (valuetoword(result, vars, persistAlloc, limits).as.word == 0) return resolvefunction(pc, funcs, stack, program, scratchAlloc, funcname, instruction_count);
			else return pc + 1;
			break;
		}

		case Opcode_Call: 
		{
			char *funcname = alloc(scratchAlloc, limits->functions_namesize);

			switch (instr->FirstOperand.Addressing) 
			{
				case addrmode_bare:
				case addrmode_true_literal:
					valuetostr(funcname, limits->functions_namesize, instr->FirstOperand.Data, limits);
					break;

				case addrmode_literal: 
				{
					char *buff = alloc(scratchAlloc, limits->functions_namesize);
					Value val = resolve_literal(buff, limits->functions_namesize, &instr->FirstOperand.Data, vars, tempAlloc, persistAlloc, limits);
					valuetostr(funcname, limits->functions_namesize, val, limits);
					break;
				}

				case addrmode_forced_eval: 
				{
					char *buff = alloc(tempAlloc, limits->functions_namesize);
					Value val = resolve_forced_eval(buff, limits->functions_namesize, &instr->FirstOperand.Data, vars, tempAlloc, persistAlloc, limits);
					valuetostr(funcname, limits->functions_namesize, val, limits);
					break;
				}

				case addrmode_pointer: 
				{
					char *buff = alloc(tempAlloc, limits->functions_namesize);
					Value v = resolve_pointer(buff, limits->functions_namesize, &instr->FirstOperand.Data, vars, tempAlloc, persistAlloc, limits);
					valuetostr(funcname, limits->functions_namesize, v, limits);
					break;
				}

				case addrmode_indirect: 
				{
					char *buff = alloc(tempAlloc, limits->functions_namesize);
					Value v = resolve_pointer(buff, limits->functions_namesize, &instr->FirstOperand.Data, vars, tempAlloc, persistAlloc, limits);
					valuetostr(funcname, limits->functions_namesize, v, limits);
					break;
				}

				default:
					error(errfatal, errcodemalformedinput, "The specified addressing mode is invalid!");
					break;
			}

			return resolvefunction(pc, funcs, stack, program, scratchAlloc, funcname, instruction_count);
			break;
		}

		case Opcode_Return: 
		{
			StackFrame frame = popframe(stack);
			return frame.return_pc;
			break;
		}

		case Opcode_End:
			if (stack->count != 0) 
			{
				StackFrame frame = popframe(stack);
				return frame.return_pc;
			}
			
			break;

		case Opcode_Escape: 
		{
			char *name = alloc(scratchAlloc, limits->escapes_namesize);

			switch (instr->FirstOperand.Addressing) 
			{
				case addrmode_bare:
				case addrmode_true_literal:
					valuetostr(name, limits->escapes_namesize, instr->FirstOperand.Data, limits);
					break;

				case addrmode_literal: 
				{
					char *buff = alloc(tempAlloc, limits->escapes_namesize);
					Value val = resolve_literal(buff, limits->escapes_namesize, &instr->FirstOperand.Data, vars, tempAlloc, persistAlloc, limits);
					valuetostr(name, limits->escapes_namesize, val, limits);
					break;
				}

				case addrmode_forced_eval: 
				{
					char *buff = alloc(tempAlloc, limits->escapes_namesize);
					Value val = resolve_forced_eval(buff, limits->escapes_namesize, &instr->FirstOperand.Data, vars, tempAlloc, persistAlloc, limits);
					valuetostr(name, limits->escapes_namesize, val, limits);
					break;
				}

				case addrmode_pointer: 
				{
					char *buff = alloc(tempAlloc, limits->instructions_varnamesize);
					Value v = resolve_pointer(buff, limits->instructions_varnamesize, &instr->FirstOperand.Data, vars, tempAlloc, persistAlloc, limits);
					valuetostr(name, limits->escapes_namesize, v, limits);
					break;
				}

				case addrmode_indirect: 
				{
					char *buff = alloc(tempAlloc, limits->instructions_varnamesize);
					Value v = resolve_pointer(buff, limits->instructions_varnamesize, &instr->FirstOperand.Data, vars, tempAlloc, persistAlloc, limits);
					valuetostr(name, limits->escapes_namesize, v, limits);
					break;
				}

				default:
					error(errfatal, errcodemalformedinput, "The specified addressing mode is invalid!");
					break;
			}

			callEscape(name, seed, ffivars, scratchAlloc, tempAlloc, persistAlloc, limits);
			break;
		}

		case Opcode_Spawn:
		{
			Task *task = scheduler_claim(scheduler, limits);
			char *buff = alloc(tempAlloc, limits->functions_namesize);

			switch(instr->FirstOperand.Addressing)
			{
				case addrmode_bare:
				case addrmode_true_literal:
					valuetostr(task->name, limits->functions_namesize, instr->FirstOperand.Data, limits);
					break;
				default:
					error(errfatal, errcodemalformedinput, "Invalid addressing mode!");
					break;
			}

			unsigned long *entry = getFunc(funcs, task->name);
			if (entry == NULL) error(errfatal, errcodemalformedinput, "The specified function does not exist!");
			task->flag[0] = '\0';

			/* second operand is not required */
			if (!(instr->SecondOperand.Addressing == addrmode_literal && instr->SecondOperand.Data.Type == type_word && instr->SecondOperand.Data.as.word == 0))
			{
				char *fbuff = alloc(tempAlloc, limits->instructions_varnamesize);

				switch(instr->SecondOperand.Addressing)
				{
					case addrmode_bare:
					case addrmode_true_literal:
						valuetostr(task->flag, limits->instructions_varnamesize, instr->SecondOperand.Data, limits);
						break;
					default:
						error(errfatal, errcodemalformedinput, "Invalid addressing mode!");
						break;
				}

				Value *flag = getVar(vars, task->flag);

				if (flag == NULL)
				{
					char *key = alloc(persistAlloc, getstrlen(task->flag) + 1);
					copystr(task->flag, key);
					flag = alloc(persistAlloc, sizeof(Value));
					putVar(vars, key, flag, persistAlloc);
				}

				*flag = (Value){ .Type = type_word, .as.word = 0 };
			}

			scheduler_start(task, *entry, persistAlloc, limits);
			return pc + 1;
		}

		case Opcode_Yield:
		{
			char *buff = alloc(tempAlloc, limits->instructions_varnamesize);
			unsigned long ms = 0;

			if (!(instr->FirstOperand.Addressing == addrmode_literal && instr->FirstOperand.Data.Type == type_word && instr->FirstOperand.Data.as.word == 0))
				switch(instr->FirstOperand.Addressing)
				{
					case addrmode_bare:
					case addrmode_true_literal:
						ms = valuetoword(instr->FirstOperand.Data, vars, persistAlloc, limits).as.word;
						break;
					default:
						error(errfatal, errcodemalformedinput, "Invalid addressing mode!");
						break;
				}

			scheduler_yield(scheduler, ms);
			return pc + 1;
		}

		/* internal opcodes */
		case Opcode_Internal_PRINT: 
		{
			char buf[32];

			print("_PRINT  (INTERNAL INSTRUCTION): ");
			print("TYPE: ");

			switch (instr->FirstOperand.Data.Type) 
			{
				case type_word:
					print("WORD,  DATA: ");
					valuetostr(buf, sizeof(buf), instr->FirstOperand.Data, limits);
					break;
				case type_sword:
					print("SWORD, DATA: ");
					valuetostr(buf, sizeof(buf), instr->FirstOperand.Data, limits);
					break;
				case type_str:
					print("STR,   DATA: ");
					copystr(instr->FirstOperand.Data.as.str, buf);
					break;
				case type_flt:
					print("FLT,   DATA: ");
					valuetostr(buf, sizeof(buf), instr->FirstOperand.Data, limits);
					break;
				case type_expr:
					error(errfatal, errcodemalformedinput, 
						"The specified addressing mode is invalid!");
					break;
				case type_null:
					error(errfatal, errcodemalformedinput, 
						"The specified addressing mode in invalid!");
					break;
			}

			if (getstrlen(buf) >= 32) error(errfatal, errcodelimitreached, 
				"The specified data is too long!");

			print(buf);
			print("\n");

			break;
		}

		case Opcode_Internal_PRINT2: 
		{
			char *buf = alloc(scratchAlloc, limits->instructions_varnamesize);
			valuetostr(buf, limits->instructions_varnamesize, instr->FirstOperand.Data, limits);
			Value *stored = getVar(vars, buf);

			if (stored == NULL) error(errfatal, errcodemalformedinput, 
				"The specified variable does not exist!");

			Value data = *stored;

			print("_PRINT2 (INTERNAL INSTRUCTION): ");
			print("TYPE: ");

			switch (data.Type) 
			{
				case type_word:
					print("WORD,  DATA: ");
					valuetostr(buf, limits->instructions_varnamesize, data, limits);
					break;

				case type_sword:
					print("SWORD, DATA: ");
					valuetostr(buf, limits->instructions_varnamesize, data, limits);
					break;

				case type_str:
					print("STR,   DATA: ");
					copystr(data.as.str, buf);
					break;

				case type_flt:
					print("FLT,   DATA: ");
					valuetostr(buf, limits->instructions_varnamesize, data, limits);
					break;

				case type_expr:
					error(errfatal, errcodemalformedinput, "Bad data type given!");
					break;

				case type_null:
					error(errfatal, errcodemalformedinput, "Bad data type given!");
					break;
			}

			print(buf);
			print("\n");

			break;
		}
		case Opcode_Internal_GETMEM: 
		{
			char buf[64];

			print("_GETMEM (INTERNAL INSTRUCTION)\n");

			print("  PERMALLOC: USED: ");
			snprint(buf, sizeof(buf), "%lu",
				getAllocatorSizeUsed(persistAlloc));
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

		case Opcode_Internal_SEGFAULT: 
		{
			print("\n");
			print("WARNING: _SEGFAULT INSTRUCTION USED: THIS PROGRAM WILL CRASH!\n");
			print("\n");
			*(volatile int *)0 = 0;
			/* we should segfault now */
			break;
		}

		case Opcode_Internal_GETSEED: 
		{
			char buff[32];

			snprint(buff, sizeof(buff), "%llu", seed);
			print("_GETSEED (INTERNAL INSTRUCTION): ");
			print(buff);
			print("\n");
		}
	}

	/* no fancy control flow needed here - incriment the pc */
	return pc + 1;
}
