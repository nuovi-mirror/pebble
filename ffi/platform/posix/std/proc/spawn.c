#define _POSIX_C_SOURCE 200809L /* XXX needed for Linux */

#include "ffi.h"
#include "limits.h"
#include "findchar.h"
#include <unistd.h>

void escape_std_proc_spawn 
(FFIvars *vars, unsigned long long seed, FFIArena *scratchAlloc, FFIArena *tempAlloc, FFIArena *persistAlloc, struct Limits *limits)
{
	char *procs = FFIallocateMemory(tempAlloc, limits->cmd_args * limits->cmd_chars);
	FFIValue *procv = FFIresolvePointer(vars, "__Escape_std.proc.spawn_ARG0", tempAlloc, limits);
	FFIconvertValueToString(procs, limits->misc_strlen, *procv, limits);
	
	char **args = FFIallocateMemory(tempAlloc, limits->cmd_args * sizeof(char *));
	char *current = procs;
	unsigned long argc = 0;

	/* parse the arguments */
	while ((current != NULL) && (argc < (limits->cmd_args - 1)))
	{
		while (*current == ' ') current++;
		if (*current == '\0') break;

		args[argc++] = current;
		char *space = findchar(current, ' ');
		if (space == NULL) break;

		*space = '\0';
		current = space + 1;
	}

	args[argc] = NULL;


	pid_t rproc = fork();
	if (rproc < 0) FFIerror(errfatal, errcodeplatformspecific, "Cannot fork(2)!");

	/* child process */
	if (rproc == 0)
	{
		execvp(args[0], args);

		/* only reached if execvp(3) failed */
		FFIerror(errfatal, errcodeplatformspecific, "Cannot run execvp(3)!");
	} else rproc = -1;

	/* parent process */

	FFIsetVariable(vars, "__Escape_std.proc.spawn_RET0", (FFIValue){ .Type = type_sword, .as.sword = (long)rproc }, persistAlloc);
}
