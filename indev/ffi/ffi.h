#ifndef FFI_H_
#define FFI_H_

#include "variables.h"

typedef struct {
	VarMap *map;
	unsigned long count;
	unsigned long max;
	unsigned long namesize;
} VMFFIvars;

void VMFFIallocateVariable
(VMFFIvars *vars, const char *varname, void *value);

void *VMFFIreadVariableUnsafe
(VMFFIvars *vars, const char *varname);

void *VMFFIreadVariable /* safer wrapper - never returns NULL, aborts instead */
(VMFFIvars *vars, const char *varname);

#endif
