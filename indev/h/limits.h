#ifndef STATE_LIMITS_H_
#define STATE_LIMITS_H_

#include "entry.h"
#include "instructions.h" /* not used directly but needed for those who use
			     these definitions */

/* allocator-related */
#define limits_allocator_temp_maxmem 8 * 1024 * 1024 /* max mem in temp allocator */
#define limits_allocator_persist_maxmem 32 * 1024 * 1024 /* max mem in persist allocator */
#define limits_allocator_scratch_maxmem 6 * 1024 * 1024 /* max mem in scratch allocator */

/* instruction-related */
#define limits_instructions_max 500000 /* max number of instructions that can be executed */
#define limits_instructions_maxbuffersize limits_instructions_max * sizeof(Instruction) /* max size of instruction memory buffer */
#define limits_instructions_initbuffersize (512 * sizeof(Instruction)) /* inital buffer size for instruction buffer */
#define limits_instructions_maxcache 512 /* max number of instructions in the instruction cache */
#define limits_instructions_varnamesize 64 /* number of chars for a variable name */

/* stack-related */
#define limits_stack_callStack_cap 512
#define limits_stack_callStack_frameSize (sizeof(StackFrame))
#define limits_stack_callStack_size (limits_stack_callStack_cap * limits_stack_callStack_frameSize)

/* function-related */
#define limits_functions_max 5000 /* max number of functiosn that can be defined */
#define limits_functions_namesize 32 /* size in bytes of a function name */

/* variable-related */
#define limits_variables_max 512 /* max number of variables at a time */

/* escape / FFI */
#define limits_escapes_namesize 32 /* max chars for an escape name */

/* misc */
#define limits_misc_maxfilebuffersize (8 * 1024 * 1024) /* max buffer size for the bytecode file */

/* specific instructions */
#define limits_instruction_new_destsize limits_instructions_varnamesize /* size in bytes of the dest buffer */
#define limits_instruction_new_datasize 32 /* size in bytes of the data buffer */

#endif

