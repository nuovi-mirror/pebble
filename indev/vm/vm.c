/*
 * include this to get the main VM entry point
 * run vmmain to run the VM
 * main itself is owned by the platform layer
 */

#include "vm.h"
#include "allocator.h"
#include "cmpstr.h"
#include "copymem.h"
#include "copystr.h"
#include "entry.h"
#include "escapes.h"
#include "evaluator.h"
#include "exitproc.h"
#include "expressions.h"
#include "ffi.h"
#include "findnewline.h"
#include "functions.h"
#include "getstrlen.h"
#include "instructionmapper.h"
#include "instructions.h"
#include "limits.h"
#include "main.h"
#include "mem.h"
#include "print.h"
#include "readfile.h"
#include "setmem.h"
#include "skipspace.h"
#include "snprint.h"
#include "values.h"
#include "variables.h"

/* used to parse an instruction operand - guesses the type
 * and addressing mode */
int parseoperand (char **cursor, InstructionOperand *out, Arena *tempAlloc,
	Arena *persistAlloc)
{
	char *p = skipspace(*cursor);
	if (p == NULL)
		return 0;

	char delim = 0;

	switch (*p) {
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
		default:
			out->Addressing = addrmode_bare;
			delim = 0;
			break;
	}

	char *start;

	if (delim) {
		p++; /* skip opening delim */
		start = p;

		while (*p != delim && *p != '\0')
			p++;

		if (*p != delim) {
			print("ERROR: VM: MAKEIR: UNDETERMINED OPERAND\n");
			exitproc(1);
		}

		*p = '\0'; /* terminate operand text */
		p++;	   /* move past it for next parse */
	} else {
		start = p;

		while (*p != ' ' && *p != '\t' && *p != '\0')
			p++;

		if (*p != '\0') {
			*p = '\0';
			p++;
		}
	}

	out->Data = guessvaluetypeorexpr(start, tempAlloc, persistAlloc);

	*cursor = p;
	return 1;
}

/* used to generate and optimize instruction intermediate
 * representation (IR) */
Instruction makeIR (char *line, Arena *tempAlloc, Arena *persistAlloc,
	InstructionMap *instructionMap)
{
	if (*line == '/' || *line == '#')
		return (struct Instruction){Opcode_Internal_NOP};
	Instruction *instrFromMap = getInstruction(instructionMap, line);
	static unsigned long current_instruction_cachesize;

	if (instrFromMap != NULL)
		return *instrFromMap;
	/* this is safe since it should have already been placed on
	 * the persistent allocator */

	Instruction instr = {0};
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

	parseoperand(&cursor, &instr.FirstOperand, tempAlloc, persistAlloc);
	parseoperand(&cursor, &instr.SecondOperand, tempAlloc, persistAlloc);
	/* XXX third operand unused for now */

	/* the instruction has been made
	 * the instruction does not exist on the mapper yet
	 * so we map it for future use */

	/* but only if we did not hit the max */
	if (current_instruction_cachesize <= limits_instructions_maxcache) {
		char *buf = alloc(persistAlloc, sizeof(Instruction));
		copymem(&instr, buf, sizeof(instr));
		putInstruction(instructionMap, line, buf, persistAlloc);
	}

	return instr;
}

/* helper to find the end of a function during definition */
unsigned long findend (Instruction *program, unsigned long instruction_count,
	unsigned long start)
{
	unsigned long depth = 1;
	unsigned long pc = start + 1;

	for (; pc < instruction_count; pc++) {
		if (program[pc].Opcode == Opcode_Func)
			depth++;
		else if (program[pc].Opcode == Opcode_End) {
			depth--;
			if (depth == 0)
				return pc;
		}
	}

	print("ERROR: HELPER FINDEND: NO MATCHING END FOUND!\n");
	exitproc(1);
}

/* helpers to resolve addressing modes */
Value resolve_literal (char *buff, unsigned long buffsize, Value *val, VarMap *vars,
	Arena *tempAlloc, Arena *persistAlloc)
{
	if (val->Type == type_expr) {
		if (val->as.expr == NULL) {
			print("ERROR: HELPER RESOLVE_LITERAL: NULL EXPRESSION!\n");
			exitproc(1);
		}

		return evalexprnode(val->as.expr, vars, persistAlloc);
	}

	const char *text;

	if (val->Type == type_str) {
		text = val->as.str;
	} else {
		valuetostr(buff, buffsize, *val);
		text = buff;
	}

	/* only actually try to parse it as an expression if it looks like one
	 * parse it normally if not */
	if (isexpression(text)) {
		int ok;
		ExprNodeData tree = str2expr(text, &ok, tempAlloc, persistAlloc);

		if (ok && tree.Type == ExprDataNode && tree.Node != NULL)
			return evalexprdata(tree, vars, persistAlloc);
	}

	return *val;
}

Value resolve_forced_eval (char *buff, unsigned long buffsize, Value *val, VarMap *vars,
	Arena *tempAlloc, Arena *persistAlloc)
{
	valuetostr(buff, buffsize, *val);
	Value *var = getVar(vars, buff);

	if (var != NULL)
		return resolve_literal(buff, buffsize, var, vars, tempAlloc, persistAlloc);

	return resolve_literal(buff, buffsize, val, vars, tempAlloc, persistAlloc);
}

unsigned long resolvefunction (unsigned long pc, FuncMap *funcs, Stack *stack,
	Instruction *program, Arena *scratchAlloc, const char *funcname,
	unsigned long instruction_count)
{
	unsigned long *retpc = getFunc(funcs, funcname);
	if (retpc == NULL) {
		print("ERROR: HELPER RESOLVEFUNC: UNKNOWN FUNCTION!\n");
		exitproc(1);
	}

	int tpos = (pc + 1 < instruction_count) && program[pc + 1].Opcode == Opcode_End;
	int scall = (stack->count != 0) &&
		    cmpstr(stack->items[stack->count - 1].funcname, funcname) == 0;

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
unsigned long interpret (Instruction *instr, Instruction *program, unsigned long long seed,
	unsigned long instruction_count, VarMap *vars, FFIvars *ffivars, Stack *stack,
	FuncMap *funcs, unsigned long pc, Arena *persistAlloc, Arena *scratchAlloc,
	Arena *tempAlloc)
{
	switch (instr->Opcode) {
		case Opcode_New: {
			char *dest = alloc(tempAlloc, limits_instructions_varnamesize);
			Value val;

			switch (instr->FirstOperand.Addressing) {
				case addrmode_bare:
					valuetostr(dest, limits_instructions_varnamesize,
						instr->FirstOperand.Data);
					break;
				case addrmode_true_literal:
					valuetostr(dest, limits_instructions_varnamesize,
						instr->FirstOperand.Data);
					break;
				case addrmode_literal: {
					char *buf = alloc(tempAlloc,
						limits_instructions_varnamesize);
					val = resolve_literal(buf,
						limits_instructions_varnamesize,
						&instr->FirstOperand.Data, vars, tempAlloc,
						persistAlloc);
					valuetostr(dest, limits_instructions_varnamesize,
						val);
					break;
				}
				case addrmode_forced_eval: {
					char *buf = alloc(tempAlloc,
						limits_instructions_varnamesize);
					val = resolve_forced_eval(buf,
						limits_instructions_varnamesize,
						&instr->FirstOperand.Data, vars, tempAlloc,
						persistAlloc);
					valuetostr(dest, limits_instructions_varnamesize,
						val);
					break;
				}

				case addrmode_pointer: {
					char *buf = alloc(tempAlloc,
						limits_instructions_varnamesize);
					valuetostr(buf, limits_instructions_varnamesize,
						instr->FirstOperand.Data);
					Value *valptr = getVar(vars, buf);
					if (valptr == NULL) {
						print("ERROR: VM: NEW: VARIABLE DOES NOT EXIST!\n");
						exitproc(1);
					}

					valuetostr(dest, limits_instructions_varnamesize,
						*valptr);

					break;
				}

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
					char *buf = alloc(tempAlloc,
						limits_instructions_varnamesize);
					val = resolve_literal(buf,
						limits_instructions_varnamesize,
						&instr->SecondOperand.Data, vars, tempAlloc,
						persistAlloc);
					break;
				}

				case addrmode_forced_eval: {
					char *buf = alloc(tempAlloc,
						limits_instructions_varnamesize);
					val = resolve_forced_eval(buf,
						limits_instructions_varnamesize,
						&instr->SecondOperand.Data, vars, tempAlloc,
						persistAlloc);
					break;
				}

				case addrmode_bare: {
					char *buf = alloc(scratchAlloc,
						limits_instructions_varnamesize);
					valuetostr(buf, limits_instructions_varnamesize,
						instr->SecondOperand.Data);
					Value *check = getVar(vars, buf);
					if (check == NULL) {
						print("ERROR: VM: INTERPRETER: NEW: VARIABLE DOES NOT EXIST!\n");
						exitproc(1);
					}

					val = *check;
					break;
				}

				case addrmode_pointer: {
					char *buf = alloc(persistAlloc,
						limits_instructions_varnamesize);
					valuetostr(buf, limits_instructions_varnamesize,
						instr->SecondOperand.Data);
					Value *valptr = getVar(vars, buf);
					if (valptr == NULL) {
						print("ERROR: VM: NEW: VARIABLE DOES NOT EXIST!\n");
						exitproc(1);
					}

					val = (Value){.Type = type_str, .as.str = buf};

					break;
				}

				default:
					print("ERROR: VM: INTERPRETER: NEW: UNKNOWN ADDRESSING MODE ON OPERAND TWO\n");
					exitproc(1);
					break;
			}

			Value *existing = getVar(vars, dest);
			if (existing != NULL)
				*existing = val;

			else {
				Value *valptr = alloc(persistAlloc, sizeof(val));
				*valptr = val;
				unsigned long namelen = getstrlen(dest);
				char *name = alloc(persistAlloc,
					namelen + 1); /* +1 for the null terminator */
				copymem(dest, name, namelen);
				name[namelen] = '\0';

				putVar(vars, name, valptr, persistAlloc);
			}

			break;
		}

		case Opcode_Func: {
			char *funcname = alloc(scratchAlloc, limits_functions_namesize);
			unsigned long *lpc = alloc(persistAlloc, sizeof(long));
			switch (instr->FirstOperand.Addressing) {
				case addrmode_bare: {
					valuetostr(funcname, limits_functions_namesize,
						instr->FirstOperand.Data);
					break;
				}

				case addrmode_true_literal: {
					valuetostr(funcname, limits_functions_namesize,
						instr->FirstOperand.Data);
					break;
				}

				case addrmode_literal: {
					char *buff = alloc(scratchAlloc,
						limits_functions_namesize);
					Value vptr = resolve_literal(buff,
						limits_functions_namesize,
						&instr->FirstOperand.Data, vars, tempAlloc,
						persistAlloc);
					valuetostr(funcname, limits_functions_namesize,
						vptr);
					break;
				}

				case addrmode_forced_eval: {
					char *buff = alloc(scratchAlloc,
						limits_functions_namesize);
					Value vptr = resolve_forced_eval(buff,
						limits_functions_namesize,
						&instr->FirstOperand.Data, vars, tempAlloc,
						persistAlloc);
					valuetostr(funcname, limits_functions_namesize,
						vptr);
					break;
				}

				case addrmode_pointer: {
					char *vstr = alloc(scratchAlloc,
						limits_functions_namesize);
					valuetostr(vstr, limits_functions_namesize,
						instr->FirstOperand.Data);
					Value *vptr = getVar(vars, vstr);
					if (vptr == NULL) {
						print("ERROR: INTERPERTER: FUNC: VARIABLE DOES NOT EXIST!\n");
						exitproc(1);
					}
					valuetostr(funcname, limits_functions_namesize,
						*vptr);
					break;
				}

				default:
					print("ERROR: INTERPRETER: FUNC: UNSUPPORTED ADDRESSING MODE!\n");
					exitproc(1);
					break;
			}

			*lpc = pc + 1; /* since we are on the func instruction */
			putFunc(funcs, funcname, lpc,
				persistAlloc); /* place the function onto the index */
			return findend(program, instruction_count, pc) +
			       1; /* find the End statement */
			break;
		}

		case Opcode_If: {
			char *funcname = alloc(scratchAlloc, limits_functions_namesize);
			Value result;

			switch (instr->FirstOperand.Addressing) {
				case addrmode_bare:
					valuetostr(funcname, limits_functions_namesize,
						instr->FirstOperand.Data);
					break;

				case addrmode_true_literal:
					valuetostr(funcname, limits_functions_namesize,
						instr->FirstOperand.Data);
					break;

				case addrmode_literal: {
					char *buff = alloc(scratchAlloc,
						limits_functions_namesize);
					Value vptr = resolve_literal(buff,
						limits_functions_namesize,
						&instr->FirstOperand.Data, vars, tempAlloc,
						persistAlloc);
					valuetostr(funcname, limits_functions_namesize,
						vptr);
					break;
				}

				case addrmode_forced_eval: {
					char *buff = alloc(scratchAlloc,
						limits_functions_namesize);
					Value vptr = resolve_forced_eval(buff,
						limits_functions_namesize,
						&instr->FirstOperand.Data, vars, tempAlloc,
						persistAlloc);
					valuetostr(funcname, limits_functions_namesize,
						vptr);
					break;
				}

				case addrmode_pointer: {
					char *vstr = alloc(scratchAlloc,
						limits_functions_namesize);
					valuetostr(vstr, limits_functions_namesize,
						instr->FirstOperand.Data);
					Value *vptr = getVar(vars, vstr);
					if (vptr == NULL) {
						print("ERROR: INTERPERTER: FUNC: VARIABLE DOES NOT EXIST!\n");
						exitproc(1);
					}
					valuetostr(funcname, limits_functions_namesize,
						*vptr);
					break;
				}

				default:
					print("ERROR: INTERPRETER: IF: UNSUPPORTED ADDRESSING MODE!\n");
					exitproc(1);
					break;
			}

			switch (instr->SecondOperand.Addressing) {
				case addrmode_literal: {
					char *buff =
						alloc(tempAlloc, limits_functions_namesize);
					result = resolve_literal(buff,
						limits_functions_namesize,
						&instr->SecondOperand.Data, vars, tempAlloc,
						persistAlloc);
					break;
				}

				case addrmode_forced_eval: {
					char *buff =
						alloc(tempAlloc, limits_functions_namesize);
					result = resolve_forced_eval(buff,
						limits_functions_namesize,
						&instr->SecondOperand.Data, vars, tempAlloc,
						persistAlloc);
					break;
				}

				/* in this case, a pointer refers to a pointer to an
				 * expression, which is compatable with the forced_eavl
				 * addressing mode, so we can just copy that */
				case addrmode_pointer: {
					char *buff =
						alloc(tempAlloc, limits_functions_namesize);
					result = resolve_forced_eval(buff,
						limits_functions_namesize,
						&instr->SecondOperand.Data, vars, tempAlloc,
						persistAlloc);
					break;
				}

				case addrmode_bare: {
					result = instr->SecondOperand.Data;
					break;
				}

				case addrmode_true_literal: {
					result = instr->SecondOperand.Data;
					break;
				}

				default:
					print("ERROR: INTERPRETER: IF: UNSUPPORTED ADDRESSING MODE!\n");
					exitproc(1);
					break;
			}
			if (valuetoword(result, vars, persistAlloc).as.word == 0)
				return resolvefunction(pc, funcs, stack, program,
					scratchAlloc, funcname, instruction_count);
			else
				return pc + 1;

			break;
		}

		case Opcode_Call: {
			char *funcname = alloc(scratchAlloc, limits_functions_namesize);

			switch (instr->FirstOperand.Addressing) {
				case addrmode_bare:
					valuetostr(funcname, limits_functions_namesize,
						instr->FirstOperand.Data);
					break;

				case addrmode_true_literal:
					valuetostr(funcname, limits_functions_namesize,
						instr->FirstOperand.Data);
					break;

				case addrmode_literal: {
					char *buff = alloc(scratchAlloc,
						limits_functions_namesize);
					Value val = resolve_literal(buff,
						limits_functions_namesize,
						&instr->FirstOperand.Data, vars, tempAlloc,
						persistAlloc);
					valuetostr(funcname, limits_functions_namesize,
						val);
					break;
				}

				case addrmode_forced_eval: {
					char *buff = alloc(scratchAlloc,
						limits_functions_namesize);
					Value val = resolve_forced_eval(buff,
						limits_functions_namesize,
						&instr->FirstOperand.Data, vars, tempAlloc,
						persistAlloc);
					valuetostr(funcname, limits_functions_namesize,
						val);
					break;
				}

				case addrmode_pointer: {
					char *buf = alloc(tempAlloc,
						limits_instructions_varnamesize);
					valuetostr(buf, limits_instructions_varnamesize,
						instr->FirstOperand.Data);
					Value *valptr = getVar(vars, buf);
					if (valptr == NULL) {
						print("ERROR: VM: CALL: VARIABLE DOES NOT EXIST!\n");
						exitproc(1);
					}

					valuetostr(funcname,
						limits_instructions_varnamesize, *valptr);

					break;
				}

				default:
					print("ERROR: INTERPRETER: CALL: UNSUPPORTED ADDRESSING MODE!\n");
					exitproc(1);
					break;
			}
			return resolvefunction(pc, funcs, stack, program, scratchAlloc,
				funcname, instruction_count);
			break;
		}

		case Opcode_Return: {
			StackFrame frame = popframe(stack);
			return frame.return_pc;
			break;
		}

		case Opcode_End:
			if (stack->count != 0) {
				StackFrame frame = popframe(stack);
				return frame.return_pc;
			}
			break;

		case Opcode_Escape: {
			char *name = alloc(scratchAlloc, limits_escapes_namesize);

			switch (instr->FirstOperand.Addressing) {
				case addrmode_bare:
					valuetostr(name, limits_escapes_namesize,
						instr->FirstOperand.Data);
					break;

				case addrmode_true_literal:
					valuetostr(name, limits_escapes_namesize,
						instr->FirstOperand.Data);
					break;

				case addrmode_literal: {
					char *buff =
						alloc(tempAlloc, limits_escapes_namesize);
					Value val = resolve_literal(buff,
						limits_escapes_namesize,
						&instr->FirstOperand.Data, vars, tempAlloc,
						persistAlloc);
					valuetostr(name, limits_escapes_namesize, val);
					break;
				}

				case addrmode_forced_eval: {
					char *buff =
						alloc(tempAlloc, limits_escapes_namesize);
					Value val = resolve_forced_eval(buff,
						limits_escapes_namesize,
						&instr->FirstOperand.Data, vars, tempAlloc,
						persistAlloc);
					valuetostr(name, limits_escapes_namesize, val);
					break;
				}

				case addrmode_pointer: {
					char *buf = alloc(tempAlloc,
						limits_instructions_varnamesize);
					valuetostr(buf, limits_instructions_varnamesize,
						instr->FirstOperand.Data);
					Value *valptr = getVar(vars, buf);
					if (valptr == NULL) {
						print("ERROR: VM: NEW: VARIABLE DOES NOT EXIST!\n");
						exitproc(1);
					}

					valuetostr(name, limits_escapes_namesize, *valptr);

					break;
				}

				default:
					print("ERROR: INTERPRETER: ESCAPE: ADDRESSING MODE IS UNSUPPORTED!\n");
					exitproc(1);
			}

			callEscape(name, seed, ffivars, scratchAlloc, tempAlloc,
				persistAlloc);
			break;
		}

		/* internal opcodes */
		case Opcode_Internal_PRINT: {
			char buf[32];

			print("_PRINT  (INTERNAL INSTRUCTION): ");
			print("TYPE: ");

			switch (instr->FirstOperand.Data.Type) {
				case type_word:
					print("WORD,  DATA: ");
					valuetostr(buf, sizeof(buf),
						instr->FirstOperand.Data);
					break;
				case type_sword:
					print("SWORD, DATA: ");
					valuetostr(buf, sizeof(buf),
						instr->FirstOperand.Data);
					break;
				case type_str:
					print("STR,   DATA: ");
					copystr(instr->FirstOperand.Data.as.str, buf);
					break;
				case type_flt:
					print("FLT,   DATA: ");
					valuetostr(buf, sizeof(buf),
						instr->FirstOperand.Data);
					break;
				case type_expr:
					print("ERROR: THIS CANNOT HANDLE EXPRESSIONS!\n");
					exitproc(1);
					break;
				case type_null:
					print("ERROR: THIS CANNOT HANDLE NULL TYPES!\n");
					exitproc(1);
					break;
			}
			if (getstrlen(buf) >= 32) {
				print("\nERROR: INTERPRETER: _PRINT (INTERNAL INSTRUCTION): DATA TOO LONG!\n");
				exitproc(1);
			}

			print(buf);
			print("\n");

			break;
		}

		case Opcode_Internal_PRINT2: {
			char *buf = alloc(scratchAlloc, limits_instructions_varnamesize);

			valuetostr(buf, limits_instructions_varnamesize,
				instr->FirstOperand.Data);
			Value *stored = getVar(vars, buf);

			if (stored == NULL) {
				print("ERROR: VM: INTERPRETER: _PRINT2: VARIABLE DOES NOT EXIST!\n");
				exitproc(1);
			}

			Value data = *stored;

			print("_PRINT2 (INTERNAL INSTRUCTION): ");
			print("TYPE: ");

			switch (data.Type) {
				case type_word:
					print("WORD,  DATA: ");
					valuetostr(buf, limits_instructions_varnamesize,
						data);
					break;
				case type_sword:
					print("SWORD, DATA: ");
					valuetostr(buf, limits_instructions_varnamesize,
						data);
					break;
				case type_str:
					print("STR,   DATA: ");
					copystr(data.as.str, buf);
					break;
				case type_flt:
					print("FLT,   DATA: ");
					valuetostr(buf, limits_instructions_varnamesize,
						data);
					break;
				case type_expr:
					print("ERROR: THIS CANNOT HANDLE EXPRESSIONS!\n");
					exitproc(1);
					break;
				case type_null:
					print("ERROR: THIS CANNOT HANDLE NULL TYPES!\n");
					exitproc(1);
					break;
			}

			print(buf);
			print("\n");

			break;
		}
		case Opcode_Internal_GETMEM: {
			char buf[sizeof(long)];

			print("_GETMEM (INTERNAL INSTRUCTION)\n");

			print("  PERMALLOC: USED: ");
			snprint(buf, sizeof(buf), "%lu",
				getAllocatorSizeUsed(persistAlloc));
			print(buf);
			setmem(buf, 0, sizeof(buf));

			print(", REMAIN: ");
			snprint(buf, sizeof(buf), "%lu",
				getAllocatorSizeRemaining(persistAlloc));
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
			snprint(buf, sizeof(buf), "%lu",
				getAllocatorSizeRemaining(tempAlloc));
			print(buf);
			setmem(buf, 0, sizeof(buf));

			print(", MAX: ");
			snprint(buf, sizeof(buf), "%lu", tempAlloc->size);
			print(buf);
			setmem(buf, 0, sizeof(buf));

			print("\n");

			print("  SCRATCH: USED: ");
			snprint(buf, sizeof(buf), "%lu",
				getAllocatorSizeUsed(scratchAlloc));
			print(buf);
			setmem(buf, 0, sizeof(buf));

			print(", REMAIN: ");
			snprint(buf, sizeof(buf), "%lu",
				getAllocatorSizeRemaining(scratchAlloc));
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

		case Opcode_Internal_SEGFAULT: {
			*(volatile int *)0 = 0;
			/* we should segfault now */
			break;
		}

		case Opcode_Internal_GETSEED: {
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

/* Args and Stack should be defined by the platform entry code
 * which will include this file */
int vmmain (Args cliargs, Stack *stack, unsigned long long seed)
{
	/* init */
	struct Arena *tempAlloc =
		initAlloc(mem_arena_tempAlloc_backing, limits_allocator_temp_maxmem);
	struct Arena *persistAlloc =
		initAlloc(mem_arena_persistAlloc_backing, limits_allocator_persist_maxmem);
	struct Arena *scratchAlloc =
		initAlloc(mem_arena_scratchAlloc_backing, limits_allocator_scratch_maxmem);
	struct Arena *IRAlloc =
		initAlloc(mem_arena_IRAlloc_backing, limits_instructions_maxbuffersize);

	/* XXX read this pls
	 * tempAlloc may be freed once after every instruction
	 * scratchAlloc may be freed in-between function calls
	 * persistAlloc may never be freed until the VM exits
	 */

	VarMap vars = initVars(limits_variables_max, persistAlloc);
	FFIvars *ffivars = alloc(persistAlloc, sizeof(FFIvars));
	FuncMap funcs = initFuncs(limits_functions_max, persistAlloc);
	InstructionMap instructionMap =
		initInstructionMap(limits_instructions_maxcache, persistAlloc);

	ffivars->map = &vars;
	ffivars->count = 0;
	ffivars->max = limits_variables_max;
	ffivars->namesize = limits_instructions_varnamesize;

	/* XXX remove this
	char *file = alloc(tempAlloc, limits_misc_maxfilebuffersize);
	char *filedata = readfile(cliargs.values[1], file,
	limits_misc_maxfilebuffersize);
	*/

	unsigned long filesize = getfilesize(cliargs.values[1]);
	if (filesize == 0) {
		print("ERROR: VM: INIT: CANNOT OPEN SPECIFIED FILE!\n");
		exitproc(1);
	}

	if (filesize + 1 > limits_misc_maxfilebuffersize) {
		print("ERROR: VM: INIT: FILE IS TOO LARGE!\n");
		exitproc(1);
	}

	char *file = alloc(tempAlloc,
		filesize + 1); /* +1 for null terminator readfile() appends */
	char *filedata = readfile(cliargs.values[1], file, filesize + 1);

	if (filedata == NULL) {
		print("ERROR: VM: INIT: CANNOT OPEN SPECIFIED FILE!\n");
		exitproc(1);
	}

	char *line;
	unsigned long current_instruction_count = 0;
	unsigned long current_instruction_buffersize = limits_instructions_initbuffersize;
	Instruction *program = alloc(IRAlloc, limits_instructions_maxbuffersize);

	/* startup */
	while ((line = findnewline(&filedata)) != NULL) {
		if (current_instruction_count >= limits_instructions_max) {
			print("ERROR: LIMITS: INSTRUCTION CAP REACHED!\n");
			exitproc(1);
		}

		char *p = skipspace(line);

		if (p == NULL)
			continue;

		program[current_instruction_count] =
			makeIR(line, tempAlloc, persistAlloc, &instructionMap);
		current_instruction_count++;
	}

	/* free buffer holding file */
	resetAllocator(tempAlloc);

	/* execution */
	unsigned long pc = 0;
	while (pc < current_instruction_count) {
		pc = interpret(&program[pc], program, seed, current_instruction_count,
			&vars, ffivars, stack, &funcs, pc, persistAlloc, scratchAlloc,
			tempAlloc);
		resetAllocator(scratchAlloc);
		resetAllocator(tempAlloc);
	}

	/* clean-up */
	freeAllocator(persistAlloc);
	freeAllocator(tempAlloc);
	freeAllocator(scratchAlloc);
	freeAllocator(IRAlloc);
	/* lfree(ffivars); XXX hack */
	freeVars(&vars);
	freeInstructionMap(&instructionMap);
	freeFuncs(&funcs);

	return 0;
}
