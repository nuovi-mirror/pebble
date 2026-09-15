#include "ffi.h"
#include "variables.h"
#include "exitproc.h"
#include "main.h"
#include "print.h"

void VMFFIallocateVariable
(VMFFIvars *vars, const char* varname, void *value)
{
	if (vars->count >= vars->max)
	{
		print("ERROR: FFI: TOO MANY VARIABLES USED!\n");
		exitproc(1);
	}

	if (vars->namesize >= getstrlen(varname))
	{
		print("ERROR: FFI: VARIABLE NAME IS TOO LONG!\n");
		exitproc(1);
	}

	if (value == NULL)
	{
		print("ERROR: FFI: VALUE IS NULL!\n");
		exitproc(1);
	}

	if (strlen(varname) == 0)
	{
		print("ERROR: FFI: NO VARIABLE NAME GIVEN!\n");
		exitproc(1);
	}

	putVar(vars->map, &varname, value);
}

void *VMFFIreadVariableUnsafe
(VMFFIvars *vars, const char* varname)
{
	if (strlen(varname) == 0)
	{
		print("ERROR: FFI: NO VARIABLE NAME GIVEN!\n");
		exitproc(1);
	}

	if (strlen(varname) > vars->namesize)
	{
		print("ERROR: FFI: VARIABLE NAME IS TOO LONG!\n");
		exitproc(1);
	}

	void *var = getVar(vars->map, &varname);
	
	/* may return NULL */
	return var;
}

void *VMFFIreadVariable /* safer wrapper */
(VMFFIvars *vars, const char* varname)
{
	void *uvar = VMFFIreadVariable(vars, varname);

	if (uvar == NULL)
	{
		print("ERROR: FFI: VARIABLE DOES NOT EXIST!\n");
		exitproc(1);
	}

	/* cannot be null anymore */
	return uvar;
}
