#ifndef VM_H_
#define VM_H_

#include "vm.h"
#include "entry.h"
#include "print.h"
#include "copystr.h"
#include "getstrlen.h"
#include "findnewline.h"
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

int parseoperand
(char **cursor, InstructionOperand *out, Arena *persistAlloc);

Instruction makeIR
(char *line, Arena *persistAlloc, InstructionMap *instructionMap);

unsigned long findend
(Instruction *program, unsigned long instruction_count, unsigned long start);

Value resolve_literal
(char *buff, unsigned long buffsize, Value *val, VarMap *vars, Arena *tempAlloc);
	
Value resolve_forced_eval
(char *buff, unsigned long buffsize, Value *val, VarMap *vars, Arena *tempAlloc);

unsigned long interpret
(Instruction *instr, Instruction *program,  unsigned long instruction_count, 
 VarMap *vars, Stack *stack, FuncMap *funcs, unsigned long pc, Arena *persistAlloc, 
 Arena *scratchAlloc, Arena *tempAlloc);

/* call this to enter the VM code */
int vmmain
(Args cliargs, Stack *stack); 

#endif
