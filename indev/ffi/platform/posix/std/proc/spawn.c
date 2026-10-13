#include "ffi.h"
#include "limits.h"
#include "findchar.h"
#include <unistd.h>

#define argcount 32

void escape_std_proc_spawn (FFIvars *vars, unsigned long long seed, FFIArena *scratchAlloc,
	FFIArena *tempAlloc, FFIArena *persistAlloc, struct Limits *limits)
{
	char *ptrbuff = FFIallocateMemory(tempAlloc, limits->instructions_varnamesize);
	char *msgbuff = FFIallocateMemory(tempAlloc, limits->instructions_varnamesize);

	FFIValue *ptr = FFIreadVariable(vars, "__Escape_std.proc.spawn_ARG0");
	FFIconvertValueToString(ptrbuff, limits->instructions_varnamesize, *ptr);
	FFIValue *msg = FFIreadVariable(vars, ptrbuff);
	FFIconvertValueToString(msgbuff, limits->instructions_varnamesize, *msg);
	
	char *args[argcount] = { NULL };
	char *current = msgbuff;
	unsigned long i = 0;

	/* parse the arguments */
	while (i < argcount - 1 && current != NULL)
	{
		args[i++] = current;
		char *space = findchar(current, ' ');

		if (space == NULL) break;
		*space = '\0';
		current = space + 1;

		/* skip many spaces */
		while (*current == ' ') current++;
	}


	pid_t rproc = fork();
	int ret = -1; /* err by default */

	if (rproc < 0)
	{
		/* fork failed */
		FFIstdoutPrint("ERROR: FFI std.proc.spawn: CANNOT RUN fork(2) (POSIX build)!\n");
		FFIexit(1);
	/* child process */
	} else if (rproc == 0)
	{
		execvp(args[0], args);

		/* only reached if execvp(3) failed */
		FFIstdoutPrint("ERROR: FFI std.proc.spawn: CANNOT RUN execvp(3) (POSIX build)!\n");
		FFIexit(1);
	} else ret = rproc; /* also parent process */

	/* parent process */

	FFIValue *packed = FFIallocateMemory(persistAlloc, sizeof(FFIValue));
	*packed = (FFIValue){ .Type = type_sword, .as.sword = ret };

	FFIallocateVariable(vars, "__Escape_std.proc.RET0", packed, persistAlloc);
}
