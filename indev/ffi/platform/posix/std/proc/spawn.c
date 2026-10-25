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
	
	char *args = FFIallocateMemory(tempAlloc, limits->cmd_args * limits->cmd_chars);
	char *current = procs;
	unsigned long i = 0;

	/* parse the arguments */
	while (i < limits->cmd_args - 1 && current != NULL)
	{
		args[i++] = *current;
		char *space = findchar(current, ' ');

		if (space == NULL) break;
		*space = '\0';
		current = space + 1;

		/* skip many spaces */
		while (*current == ' ') current++;
	}


	pid_t rproc = fork();
	int ret = -1; /* err by default */

	if (rproc < 0) FFIerror(errfatal, errcodeplatformspecific, "Cannot fork(2)!");

	/* child process */
	else if (rproc == 0)
	{
		execvp(&args[0], &args);

		/* only reached if execvp(3) failed */
		FFIerror(errfatal, errcodeplatformspecific, "Cannot run execvp(3)!");
	} else ret = rproc; /* also parent process */

	/* parent process */

	FFIValue *packed = FFIallocateMemory(persistAlloc, sizeof(FFIValue));
	*packed = (FFIValue){ .Type = type_sword, .as.sword = ret };

	FFIallocateVariable(vars, "__Escape_std.proc.spawn_RET0", packed, persistAlloc);
}
