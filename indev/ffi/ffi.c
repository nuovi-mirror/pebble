#include "ffi.h"
#include "variables.h"
#include "values.h"
#include "getstrlen.h"
#include "exitproc.h"
#include "main.h"
#include "print.h"

void FFIallocateVariable
(FFIvars *vars, const char* varname, void *value, FFIArena *persistAlloc)
{
	if (vars->count >= vars->max)
	{
		print("ERROR: FFI: TOO MANY VARIABLES USED!\n");
		exitproc(1);
	}

	if (getstrlen(varname) > vars->namesize)
	{
		print("ERROR: FFI: VARIABLE NAME IS TOO LONG!\n");
		exitproc(1);
	}

	if (value == NULL)
	{
		print("ERROR: FFI: VALUE IS NULL!\n");
		exitproc(1);
	}

	if (getstrlen(varname) == 0)
	{
		print("ERROR: FFI: NO VARIABLE NAME GIVEN!\n");
		exitproc(1);
	}

	putVar(vars->map, varname, value, persistAlloc);
}

void *FFIreadVariableUnsafe
(FFIvars *vars, const char* varname)
{
	if (getstrlen(varname) == 0)
	{
		print("ERROR: FFI: NO VARIABLE NAME GIVEN!\n");
		exitproc(1);
	}

	if (getstrlen(varname) > vars->namesize)
	{
		print("ERROR: FFI: VARIABLE NAME IS TOO LONG!\n");
		exitproc(1);
	}

	void *var = getVar(vars->map, varname);
	
	/* may return NULL */
	return var;
}

void *FFIreadVariable /* safer wrapper */
(FFIvars *vars, const char* varname)
{
	void *uvar = FFIreadVariableUnsafe(vars, varname);

	if (uvar == NULL)
	{
		print("ERROR: FFI: VARIABLE DOES NOT EXIST!\n");
		exitproc(1);
	}

	/* cannot be null anymore */
	return uvar;
}

int FFIconvertValueToString
(char *buff, unsigned long buffsize, FFIValue v)
{ return valuetostr(buff, buffsize, v); }

FFIValue FFIconvertValueToWord
(FFIValue v, FFIvars *vars, FFIArena *persistAlloc)
{ return valuetoword(v, vars->map, persistAlloc); }

void FFIstdoutPrint
(char *msg)
{ return print(msg); }

void FFIexit
(int stat)
{ return exitproc(stat); }

unsigned long long FFImix64
(unsigned long long x)
{
    x ^= x >> 30;
    x *= 0xBF58476D1CE4E5B9ULL;
    x ^= x >> 27;
    x *= 0x94D049BB133111EBULL;
    x ^= x >> 31;

    return x;
}

void *FFIallocateMemory
(FFIArena *arena, unsigned long size)
{ return alloc(arena, size); }
