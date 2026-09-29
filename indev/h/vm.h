#ifndef h_vm_
#define h_vm_

#include "instructions.h"
#include "variables.h"
#include "ffi.h"
#include "main.h"
#include "functions.h"
#include "instructionmapper.h"
#include "mem.h"
#include "limits.h"

/* stuff for vmmain() */
unsigned long interpret(Instruction *instr, Instruction *program, unsigned long long seed, 
	unsigned long instruction_count, struct Limits *limits, VarMap *vars, FFIvars *ffivars, 
	Stack *stack, FuncMap *funcs, unsigned long pc, Arena *persistAlloc, 
	Arena *scratchAlloc, Arena *tempAlloc);

Instruction makeIR (char *line, struct Limits *limits, Arena *tempAlloc, Arena *persistAlloc,
        InstructionMap *instructionMap);

unsigned long getprogram (char *file, unsigned long maxsize, Args *cliargs);

/* call this to enter the VM code */
int vmmain (Args cliargs, Stack *stack, unsigned long long seed, 
	struct Memory *mem, struct Limits *limits);
#endif
