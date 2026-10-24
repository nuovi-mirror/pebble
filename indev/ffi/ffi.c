#include "ffi.h"

#include "exitproc.h"
#include "getstrlen.h"
#include "main.h"
#include "print.h"
#include "values.h"
#include "variables.h"

void FFIallocateVariable 
(FFIvars *vars, const char *varname, void *value, FFIArena *persistAlloc)
{
	if (vars->count >= vars->max) error(errfatal, errcodelimitreached, "Too many FFI variables used!");
	if (getstrlen(varname) > vars->namesize) error(errfatal, errcodelimitreached, "FFI variable name is too long!");
	if (value == NULL) error(errfatal, errcodemalformedinput, "FFI variable is NULL!");
	if (getstrlen(varname) == 0) error(errfatal, errcodemalformedinput, "FFI variable name is empty!");

	putVar(vars->map, varname, value, persistAlloc);
}

void *FFIreadVariableUnsafe 
(FFIvars *vars, const char *varname)
{
	if (getstrlen(varname) == 0) error(errfatal, errcodemalformedinput, "FFI variable name is empty!");
	if (getstrlen(varname) > vars->namesize) error(errfatal, errcodelimitreached, "FFI variable name is too long!");

	void *var = getVar(vars->map, varname);

	/* may return NULL */
	return var;
}

void *FFIreadVariable /* safer wrapper */
(FFIvars *vars, const char *varname)
{
	void *uvar = FFIreadVariableUnsafe(vars, varname);
	if (uvar == NULL) error(errfatal, errcodemalformedinput, "FFI variable does not exist!");

	/* cannot be null anymore */
	return uvar;
}

int FFIconvertValueToString 
(char *buff, unsigned long buffsize, FFIValue v, struct Limits *limits)
{ return valuetostr(buff, buffsize, v, limits); }

FFIValue FFIconvertValueToWord 
(FFIValue v, FFIvars *vars, FFIArena *persistAlloc, struct Limits *limits)
{ return valuetoword(v, vars->map, persistAlloc, limits); }

void FFIstdoutPrint 
(char *msg)
{ print(msg); }

void FFIexit 
(int stat)
{ exitproc(stat); }

void *FFIallocateMemory 
(FFIArena *arena, unsigned long size)
{ return alloc(arena, size); }

void FFIerror
(unsigned int errtype, unsigned int errcode, const char *msg)
{ error(errtype, errcode, msg); }

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


