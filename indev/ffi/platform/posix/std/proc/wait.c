#include "ffi.h"
#include "limits.h"
#include <unistd.h>
#include <sys/wait.h>

/* XXX this is different from the older Zig one, since that one sucked 
 *
 * RET0 - either the signal it was killed with or the exit status 
 * RET1 - 0 if it was killed with a signal, 1 if not */

void escape_std_proc_wait (FFIvars *vars, unsigned long long seed, FFIArena *scratchAlloc,
	FFIArena *tempAlloc, FFIArena *persistAlloc, struct Limits *limits)
{
	char *ptrbuff = FFIallocateMemory(tempAlloc, limits->instructions_varnamesize);
	FFIValue *ptr = FFIreadVariable(vars, "__Escape_std.proc.wait_ARG0");
	FFIconvertValueToString(ptrbuff, limits->instructions_varnamesize, *ptr);
	FFIValue *msg = FFIreadVariable(vars, ptrbuff);

	int pid = (int)msg->as.sword;
	int status;

	/* wait on the child process */
	if (waitpid(pid, &status, 0) == -1) FFIerror(errfatal, errcodegeneric, "Cannot waitpid(2)!\n");

	FFIValue *exitmarker = FFIallocateMemory(persistAlloc, sizeof(FFIValue));
	FFIValue *exitcode = FFIallocateMemory(persistAlloc, sizeof(FFIValue));

	if (WIFSIGNALED(status)) /* killed by a signal */
	{
		*exitmarker = (FFIValue){ .Type = type_word, .as.word = 0 }; /* killed with signal */
		*exitcode = (FFIValue){ .Type = type_sword, .as.sword = status }; /* exit signal */
		FFIallocateVariable(vars, "__Escape_std.proc.wait_RET0", &exitcode, persistAlloc);
		FFIallocateVariable(vars, "__Escape_std.proc.wait_RET1", &exitcode, persistAlloc);
	} else /* normal exit */ 
	{
		*exitmarker = (FFIValue){ .Type = type_word, .as.word = 1 }; /* not killed with signal */
		*exitcode = (FFIValue){ .Type = type_sword, .as.sword = status }; /* exit status */
		FFIallocateVariable(vars, "__Escape_std.proc.wait_RET0", &exitcode, persistAlloc);
		FFIallocateVariable(vars, "__Escape_std.proc.wait_RET1", &exitcode, persistAlloc);
	}
}
