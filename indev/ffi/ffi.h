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
(FFIvars *vars, const char *varname, void *value);

void *FFIreadVariableUnsafe
(FFIvars *vars, const char *varname);

void *FFIreadVariable /* safer wrapper - never returns NULL, aborts instead */
(FFIvars *vars, const char *varname);

int FFIconvertValueToString
(char *buff, unsigned long buffsize, FFIValue v);

void FFIstdoutPrint
(char *msg);

void *FFIallocateMemory
(FFIArena *arena, unsigned long size);

#endif
