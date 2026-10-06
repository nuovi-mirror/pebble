#ifndef h_limits_
#define h_limits_

struct Limits {
	/* allocator-related */
	unsigned long allocator_temp_maxmem; /* max mem in temp allocator */
	unsigned long allocator_persist_maxmem; /* max mem in persist allocator */
	unsigned long allocator_scratch_maxmem; /* max mem in scratch allocator */
	unsigned long allocator_IRAlloc_maxmem; /* max mem in scratch allocator */

	/* instruction-related */
	unsigned long instructions_max; /* max number of instructions that can be executed */
	unsigned long instructions_maxbuffersize; /* max size for the instruction buffer */
	unsigned long instructions_initbuffersize; /* inital buffer size for instruction buffer */
	unsigned long instructions_maxcache; /* max number of instructions in the instruction cache */
	unsigned long instructions_varnamesize; /* number of chars for a variable name */

	/* stack-related */
	unsigned long stack_callStack_cap;
	unsigned long stack_callStack_frameSize;
	unsigned long stack_callStack_size;

	/* function-related */
	unsigned long functions_max; /* max number of functiosn that can be defined */
	unsigned long functions_namesize; /* size in bytes of a function name */

	/* variable-related */
	unsigned long variables_max; /* max number of variables at a time */

	/* escape / FFI */
	unsigned long escapes_namesize; /* max chars for an escape name */

	/* misc */
	unsigned long misc_maxfilebuffersize; /* max buffer size for the bytecode file */
	unsigned long misc_vmargmaxdigits; /* max number of arg digits, eg '2' for up to '99' args */

	/* specific instructions */
	unsigned long instruction_new_destsize; /* size in bytes of the new opcode dest buffer */
	unsigned long instruction_new_datasize; /* size in bytes of the new opcode data buffer */
};

#endif
