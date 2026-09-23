#ifndef FFI_H_
#define FFI_H_

#include "variables.h"
#include "values.h"
#include "allocator.h"

typedef struct {
	VarMap *map;
	unsigned long count;
	unsigned long max;
	unsigned long namesize;
} FFIvars;

typedef Value FFIValue;
typedef Arena FFIArena;

void FFIallocateVariable
(FFIvars *vars, const char *varname, void *value, FFIArena *persistAlloc);

void *FFIreadVariableUnsafe
(FFIvars *vars, const char *varname);

void *FFIreadVariable /* safer wrapper - never returns NULL, aborts instead */
(FFIvars *vars, const char *varname);

int FFIconvertValueToString
(char *buff, unsigned long buffsize, FFIValue v);

FFIValue FFIconvertValueToWord
(FFIValue v, FFIvars *vars, FFIArena *persistAlloc);

void FFIstdoutPrint
(char *msg);

void FFIexit
(int stat);

/* use this to get a random seed for your library
 * FFImix64(seed ^ some_number);
 * some number should probably be say the last 8
 * characters of your library name 
 * the seed variable is given to you by the VM */
unsigned long long FFImix64
(unsigned long long x);

void *FFIallocateMemory
(FFIArena *arena, unsigned long size);

#endif
