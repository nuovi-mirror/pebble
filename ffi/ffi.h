#ifndef FFI_H_
#define FFI_H_

#include "main.h" /* needed for null definition */
#include "allocator.h"
#include "values.h"
#include "variables.h"

typedef struct {
	VarMap *map;
	unsigned long count;
	unsigned long max;
	unsigned long namesize;
} FFIvars;

typedef Value FFIValue;
typedef Arena FFIArena;

void *FFIallocateMemory (FFIArena *arena, unsigned long size);
void FFIallocateVariable (FFIvars *vars, const char *varname, void *value, FFIArena *persistAlloc);
void *FFIreadVariableUnsafe (FFIvars *vars, const char *varname);
FFIValue *FFIreadVariable (FFIvars *vars, const char *varname); /* wrapper - aborts instead of returning NULL */
void FFIsetVariable (FFIvars *vars, const char *varname, FFIValue value, FFIArena *persistAlloc);

void *FFIresolvePointerUnsafe (FFIvars *vars, const char *varname, FFIArena *tempAlloc, struct Limits *limits);
FFIValue *FFIresolvePointer (FFIvars *vars, const char *varname, FFIArena *tempAlloc, struct Limits *limits);

int FFIconvertValueToString (char *buff, unsigned long buffsize, FFIValue v, struct Limits *limits);
FFIValue FFIconvertValueToWord (FFIValue v, FFIvars *vars, FFIArena *persistAlloc, struct Limits *limits);
FFIValue FFIconvertValueToSword (FFIValue v, FFIvars *vars, FFIArena *persistAlloc, struct Limits *limits);

void FFIstdoutPrint (const char *msg);
void FFIstdoutPrintValue (FFIValue *var, FFIArena *tempAlloc, struct Limits *limits);

void FFIexit (int stat); /* XXX depreciated */
void FFIerror (unsigned int severity, unsigned int status, const char *msg);

#endif
