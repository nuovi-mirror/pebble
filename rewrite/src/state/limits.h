#ifndef STATE_LIMITS_H_
#define STATE_LIMITS_H_

#include "instructions.h"

#define limits_allocator_temp_maxmem 2 * 1024 * 1024 /* max mem in temp allocator */
#define limits_allocator_persist_maxmem 8 * 1024 * 1024 /* max mem in persist allocator */
#define limits_allocator_scratch_maxmem 2 * 1024 * 1024 /* max mem in scratch allocator */

#define limits_instructions_max 512 /* max number of instructions that can be executed */
#define limits_instructions_maxbuffersize 4096 * sizeof(Instruction)
			/* max size of instruction memory buffer */
#define limits_instructions_initbuffersize 512 * sizeof(Instruction)
			/* inital buffer size for instruction buffer */

#define limits_variables_max 512 /* max number of variables at a time */

#define limits_misc_maxfilebuffersize 512 * 1024 /* max buffer size for the bytecode file */

#endif

