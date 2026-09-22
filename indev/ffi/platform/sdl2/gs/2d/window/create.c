#include "ffi.h"
#include "limits.h"
#include "SDL2/SDL.h"

void escape_gs_2d_window_create
(FFIvars *vars, FFIArena *scratchAlloc, 
 FFIArena *tempAlloc, FFIArena *persistAlloc) 
{
	/* get arguments (yes) */
	char *parg0ptr = FFIallocateMemory(tempAlloc, limits_instructions_varnamesize);
	FFIValue *parg0v = FFIreadVariable(vars, "__Escape_gs.2d.window.create_ARG0");
	FFIconvertValueToString(parg0ptr, limits_instructions_varnamesize, *parg0v);
	FFIValue *arg0v = FFIreadVariable(vars, parg0ptr);	

	char *parg1ptr = FFIallocateMemory(tempAlloc, limits_instructions_varnamesize);
	FFIValue *parg1v = FFIreadVariable(vars, "__Escape_gs.2d.window.create_ARG1");
	FFIconvertValueToString(parg1ptr, limits_instructions_varnamesize, *parg1v);
	FFIValue *arg1v = FFIreadVariable(vars, parg1ptr);

	char *parg2ptr = FFIallocateMemory(tempAlloc, limits_instructions_varnamesize);
	FFIValue *parg2v = FFIreadVariable(vars, "__Escape_gs.2d.window.create_ARG2");
	FFIconvertValueToString(parg2ptr, limits_instructions_varnamesize, *parg2v);
	FFIValue *arg2v = FFIreadVariable(vars, parg2ptr);

	char *parg3ptr = FFIallocateMemory(tempAlloc, limits_instructions_varnamesize);
	FFIValue *parg3v = FFIreadVariable(vars, "__Escape_gs.2d.window.create_ARG3");
	FFIconvertValueToString(parg3ptr, limits_instructions_varnamesize, *parg3v);
	FFIValue *arg3v = FFIreadVariable(vars, parg3ptr);

	char *parg4ptr = FFIallocateMemory(tempAlloc, limits_instructions_varnamesize);
	FFIValue *parg4v = FFIreadVariable(vars, "__Escape_gs.2d.window.create_ARG4");
	FFIconvertValueToString(parg4ptr, limits_instructions_varnamesize, *parg4v);
	FFIValue *arg4v = FFIreadVariable(vars, parg4ptr);

	/* convert argument 0 into a string so we can use it */
	char *wname = alloc(persistAlloc, limits_instructions_varnamesize);
	FFIconvertValueToString(wname, limits_instructions_varnamesize, *arg0v);

	/* convert the rest of them into words we can use */
	FFIValue arg1w = FFIconvertValueToWord(*arg1v, vars, persistAlloc);
	FFIValue arg2w = FFIconvertValueToWord(*arg2v, vars, persistAlloc);
	FFIValue arg3w = FFIconvertValueToWord(*arg3v, vars, persistAlloc);
	FFIValue arg4w = FFIconvertValueToWord(*arg4v, vars, persistAlloc);

	/* XXX should already be done by someone else
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		FFIstdoutPrint(SDL_GetError());
		FFIexit(1);
	}
	*/

	FFIstdoutPrint("Creating window\n");

	SDL_Window *win = SDL_CreateWindow(
			wname,
			arg1w.as.word, 
			arg2w.as.word, 
			arg3w.as.word, 
			arg4w.as.word,
			0);
	if (win == NULL)
	{
		FFIstdoutPrint
			("ERROR: FFI: gs.2d.window.create: FAILED TO CREATE WINDOW!\n");
		FFIexit(1);
	}

	FFIstdoutPrint("Created window\n");

	FFIallocateVariable(
			vars, 
			"__Escape_gs.2d.window.create_RET0", 
			win, 
			persistAlloc);

}
