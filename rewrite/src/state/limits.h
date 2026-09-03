#ifndef STATE_LIMITS_H_
#define STATE_LIMITS_H_

#include "instructions.h"

/* allocator-related */
#define limits_allocator_temp_maxmem 8 * 1024 * 1024 /* max mem in temp allocator */
#define limits_allocator_persist_maxmem 128 * 1024 * 1024 /* max mem in persist allocator */
#define limits_allocator_scratch_maxmem 6 * 1024 * 1024 /* max mem in scratch allocator */

/* instruction-related */
#define limits_instructions_max 500000 /* max number of instructions that can be executed */
#define limits_instructions_maxbuffersize limits_instructions_max * sizeof(Instruction)
			/* max size of instruction memory buffer */
#define limits_instructions_initbuffersize 512 * sizeof(Instruction)
			/* inital buffer size for instruction buffer */

/* variable-related */
#define limits_variables_max 512 /* max number of variables at a time */

/* misc */
#define limits_misc_maxfilebuffersize 8 * 1024 * 1024 /* max buffer size for the bytecode file */

#endif

