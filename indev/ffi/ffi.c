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

	/* XXX may return NULL */
	return var;
}

FFIValue *FFIreadVariable /* safer wrapper */
(FFIvars *vars, const char *varname)
{
	void *uvar = FFIreadVariableUnsafe(vars, varname);
	if (uvar == NULL) FFIerror(errfatal, errcodemalformedinput, "FFI variable does not exist!");

	/* cannot be null anymore */
	return uvar;
}

/* set a variable to a value, eg. an escape's return value. Values are copied by value, 
 * so if the variable already exists it is updated in place. allocating a new one on every 
 * call would eat the persistent arena, which is never freed */
void FFIsetVariable 
(FFIvars *vars, const char *varname, FFIValue value, FFIArena *persistAlloc)
{
	FFIValue *existing = FFIreadVariableUnsafe(vars, varname);

	if (existing != NULL) 
	{
		*existing = value;
		return;
	}

	FFIValue *created = FFIallocateMemory(persistAlloc, sizeof(FFIValue));
	*created = value;
	FFIallocateVariable(vars, varname, created, persistAlloc);
}

void *FFIresolvePointerUnsafe 
(FFIvars *vars, const char *varname, FFIArena *tempAlloc, struct Limits *limits)
{
	if (getstrlen(varname) == 0) error(errfatal, errcodemalformedinput, "FFI variable name is empty!");
	if (getstrlen(varname) > vars->namesize) error(errfatal, errcodelimitreached, "FFI variable name is too long!");

	void *var = getVar(vars->map, varname);
	if (var == NULL) return NULL;

	char *buffer = FFIallocateMemory(tempAlloc, limits->misc_strlen);
	FFIValue *value = (FFIValue *)var;
	FFIconvertValueToString(buffer, limits->misc_strlen, *value, limits);

	void *var2 = getVar(vars->map, buffer);
	if (var2 == NULL) return NULL;

	/* XXX may return NULL */
	return var2;
}

FFIValue *FFIresolvePointer /* safer wrapper */
(FFIvars *vars, const char *varname, FFIArena *tempAlloc, struct Limits *limits)
{
	void *uvar = FFIresolvePointerUnsafe(vars, varname, tempAlloc, limits);
	if (uvar == NULL) FFIerror(errfatal, errcodemalformedinput, "FFI variable does not exist!");

	/* cannot be null anymore */
	return uvar;
}

int FFIconvertValueToString 
(char *buff, unsigned long buffsize, FFIValue v, struct Limits *limits)
{ return valuetostr(buff, buffsize, v, limits); }

FFIValue FFIconvertValueToWord 
(FFIValue v, FFIvars *vars, FFIArena *persistAlloc, struct Limits *limits)
{ return valuetoword(v, vars->map, persistAlloc, limits); }

FFIValue FFIconvertValueToSword 
(FFIValue v, FFIvars *vars, FFIArena *persistAlloc, struct Limits *limits)
{ return valuetosword(v, vars->map, persistAlloc, limits); }

void FFIstdoutPrint 
(const char *msg)
{ print(msg); }

void FFIstdoutPrintValue
(FFIValue *var, FFIArena *tempAlloc, struct Limits *limits)
{
	char *buffer = FFIallocateMemory(tempAlloc, limits->misc_strlen);
	FFIconvertValueToString(buffer, limits->misc_strlen, *var, limits);
	FFIstdoutPrint(buffer);
}

/* XXX depreciated */
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
